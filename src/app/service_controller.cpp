#include "visiondarts/app/service_controller.hpp"

#include <chrono>
#include <thread>
#include <utility>

#include "visiondarts/core/json_utils.hpp"
#include "visiondarts/vision/calibration.hpp"

namespace visiondarts
{
ServiceController::ServiceController(AppConfig config)
    : config_(std::move(config))
    , engine_(config_.pipeline)
    , live_engine_(config_.pipeline)
    , publisher_(config_.backend)
{
    publisher_.set_error_callback([this](const std::string& message) {
        set_last_error(message);
    });
    cameras_configured_ = config_.execution.mode == "live"
        ? static_cast<int>(config_.cameras.empty() ? 1 : config_.cameras.size())
        : 1;
    publisher_.start();
}

ServiceController::~ServiceController()
{
    stop();
    live_camera_source_.close();
    publisher_.stop();
}

nlohmann::json ServiceController::start()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ == ServiceState::Running)
        {
            return nlohmann::json{{"accepted", true}, {"state", to_string(state_)}, {"message", "Le service traite deja des scenarios."}};
        }
    }

    if (worker_.joinable())
    {
        worker_.join();
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_requested_ = false;
        state_ = ServiceState::Running;
        last_error_.reset();
    }
    worker_ = std::thread(&ServiceController::worker_loop, this);
    return nlohmann::json{{"accepted", true}, {"state", to_string(ServiceState::Running)}};
}

nlohmann::json ServiceController::stop()
{
    stop_requested_ = true;
    if (worker_.joinable())
    {
        worker_.join();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ != ServiceState::Error)
    {
        state_ = ServiceState::Idle;
    }
    current_scenario_.clear();
    live_camera_source_.close();
    cameras_opened_ = 0;
    return nlohmann::json{{"accepted", true}, {"state", to_string(state_)}};
}

nlohmann::json ServiceController::reset_reference()
{
    if (config_.execution.mode != "live")
    {
        return nlohmann::json{
            {"accepted", false},
            {"error", "unsupported_in_offline_mode"},
            {"message", "Le mode offline ne gere pas de recapture de reference live."},
        };
    }

    try
    {
        ensure_live_camera_source_open();
        capture_live_reference();
        return nlohmann::json{{"accepted", true}, {"state", to_string(state_)}, {"references", live_references_.size()}};
    }
    catch (const std::exception& exception)
    {
        set_last_error(exception.what());
        return nlohmann::json{{"accepted", false}, {"error", exception.what()}};
    }
}

nlohmann::json ServiceController::healthcheck() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return nlohmann::json{
        {"service", "vision"},
        {"status", last_error_.has_value() ? "degraded" : "ok"},
        {"running", state_ == ServiceState::Running},
        {"state", to_string(state_)},
        {"mode", config_.execution.mode},
        {"cameras_configured", config_.execution.mode == "live" ? cameras_configured_ : 1},
        {"cameras_opened", config_.execution.mode == "live" ? cameras_opened_ : 1},
        {"calibration_loaded", calibration_loaded_},
        {"current_scenario", current_scenario_},
        {"last_error", last_error_.has_value() ? nlohmann::json(*last_error_) : nlohmann::json(nullptr)},
    };
}

nlohmann::json ServiceController::apply_calibration_payload(const nlohmann::json& payload)
{
    try
    {
        const int camera_id = payload.at("camera_id").get<int>();
        std::vector<cv::Point2d> points_image;
        std::vector<cv::Point2d> points_board;
        for (const auto& item : payload.at("points_image"))
        {
            points_image.emplace_back(item.at("x").get<double>(), item.at("y").get<double>());
        }
        for (const auto& item : payload.at("points_board"))
        {
            points_board.emplace_back(item.at("x").get<double>(), item.at("y").get<double>());
        }

        const CalibrationData calibration = compute_calibration(
            camera_id,
            points_image,
            points_board,
            payload.value("offset_angle_deg", 0.0));

        {
            std::lock_guard<std::mutex> lock(mutex_);
            calibration_overrides_[camera_id] = calibration;
            calibration_loaded_ = true;
        }

        if (config_.execution.mode == "live")
        {
            CalibrationStore::save_json(calibration_path_for_camera(camera_id), calibration);
            std::lock_guard<std::mutex> lock(mutex_);
            live_calibrations_[camera_id] = calibration;
        }

        return nlohmann::json{{"accepted", true}, {"camera_id", camera_id}};
    }
    catch (const std::exception& exception)
    {
        set_last_error(exception.what());
        return nlohmann::json{{"accepted", false}, {"error", exception.what()}};
    }
}

void ServiceController::worker_loop()
{
    if (config_.execution.mode == "live")
    {
        live_worker_loop();
        return;
    }

    replay_worker_loop();
}

void ServiceController::replay_worker_loop()
{
    try
    {
        const auto scenario_paths = frame_source_.list_scenarios(config_.execution.scenario_root);
        for (const auto& scenario_path : scenario_paths)
        {
            if (stop_requested_)
            {
                break;
            }

            ReplayScenario scenario = frame_source_.load_scenario(scenario_path);
            {
                std::lock_guard<std::mutex> lock(mutex_);
                current_scenario_ = scenario.scenario_config.name;
            }

            OfflineRunOptions options;
            options.compare_expected = false;
            options.save_debug = config_.execution.debug_save_intermediates;
            options.debug_output_root = config_.execution.debug_output_root;
            options.allow_single_source = config_.execution.allow_single_source;
            options.shot_sequence = shot_sequence_++;
            options.calibration_override = find_override(scenario.scenario_config.camera_id);

            const OfflineRunResult result = engine_.run_scenario(scenario, options);
            publisher_.enqueue_event(result.actual_event);

            {
                std::lock_guard<std::mutex> lock(mutex_);
                const std::string event = result.actual_event.value("event", std::string{});
                const std::string code = result.actual_event.value("code", std::string{});
                calibration_loaded_ = event != "calibration_required"
                    && !(event == "vision_error" && code == "calibration_load_failure");
            }
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ != ServiceState::Error)
        {
            state_ = ServiceState::Idle;
        }
        current_scenario_.clear();
    }
    catch (const std::exception& exception)
    {
        VisionError error;
        error.timestamp_ms = unix_timestamp_ms();
        error.code = "offline_worker_failure";
        error.message = exception.what();
        publisher_.enqueue_event(error);
        set_last_error(exception.what());
    }
}

void ServiceController::live_worker_loop()
{
    try
    {
        ensure_live_camera_source_open();
        load_live_calibrations();
        capture_live_reference();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            current_scenario_ = "live";
        }

        while (!stop_requested_)
        {
            const auto frames = live_camera_source_.capture_frames();
            if (frames.empty())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(config_.live.loop_sleep_ms));
                continue;
            }

            const double change_score = live_engine_.frame_change_score(live_references_, frames);
            if (change_score < config_.live.shot_change_threshold)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(config_.live.loop_sleep_ms));
                continue;
            }

            std::vector<LiveCameraFrame> stabilized_frames = frames;
            for (int index = 0; index < config_.live.stabilization_frames && !stop_requested_; ++index)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(config_.live.loop_sleep_ms));
                const auto next_frames = live_camera_source_.capture_frames();
                if (!next_frames.empty())
                {
                    stabilized_frames = next_frames;
                }
            }

            LiveRunOptions options;
            options.allow_single_source = config_.execution.allow_single_source;
            options.shot_sequence = shot_sequence_++;
            const nlohmann::json event = live_engine_.process_shot(
                live_references_,
                stabilized_frames,
                live_calibrations_,
                options);
            publisher_.enqueue_event(event);

            if (event.value("event", std::string{}) == "shot_detected")
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(config_.live.min_ms_between_shots));
                capture_live_reference();
            }
        }

        std::lock_guard<std::mutex> lock(mutex_);
        if (state_ != ServiceState::Error)
        {
            state_ = ServiceState::Idle;
        }
        current_scenario_.clear();
    }
    catch (const std::exception& exception)
    {
        VisionError error;
        error.timestamp_ms = unix_timestamp_ms();
        error.code = "live_worker_failure";
        error.message = exception.what();
        publisher_.enqueue_event(error);
        set_last_error(exception.what());
    }
}

void ServiceController::ensure_live_camera_source_open()
{
    if (!live_camera_source_.is_open())
    {
        std::vector<LiveCameraConfig> cameras = config_.cameras;
        if (cameras.empty())
        {
            cameras.push_back(LiveCameraConfig{});
        }
        live_camera_source_.open(cameras);
    }

    std::lock_guard<std::mutex> lock(mutex_);
    cameras_configured_ = live_camera_source_.configured_count();
    cameras_opened_ = live_camera_source_.opened_count();
}

void ServiceController::load_live_calibrations()
{
    std::unordered_map<int, CalibrationData> calibrations;
    std::vector<int> missing_camera_ids;
    const std::vector<LiveCameraConfig> cameras = config_.cameras.empty()
        ? std::vector<LiveCameraConfig>{LiveCameraConfig{}}
        : config_.cameras;

    for (const auto& camera : cameras)
    {
        if (!camera.enabled)
        {
            continue;
        }

        if (const auto override = find_override(camera.camera_id))
        {
            calibrations[camera.camera_id] = *override;
            continue;
        }

        try
        {
            calibrations[camera.camera_id] = CalibrationStore::load(camera.calibration_path);
        }
        catch (const CalibrationMissingError&)
        {
            missing_camera_ids.push_back(camera.camera_id);
        }
    }

    if (!missing_camera_ids.empty())
    {
        CalibrationRequired required;
        required.timestamp_ms = unix_timestamp_ms();
        required.missing_camera_ids = missing_camera_ids;
        publisher_.enqueue_event(required);
        std::lock_guard<std::mutex> lock(mutex_);
        live_calibrations_ = std::move(calibrations);
        calibration_loaded_ = false;
        throw std::runtime_error("Calibration live manquante pour au moins une camera.");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    live_calibrations_ = std::move(calibrations);
    calibration_loaded_ = !live_calibrations_.empty();
}

void ServiceController::capture_live_reference()
{
    std::unordered_map<int, cv::Mat> references;
    for (int index = 0; index < config_.live.reference_stability_frames && !stop_requested_; ++index)
    {
        const auto frames = live_camera_source_.capture_frames();
        for (const auto& frame : frames)
        {
            references[frame.config.camera_id] = frame.image.clone();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(config_.live.loop_sleep_ms));
    }

    if (references.empty())
    {
        throw std::runtime_error("Impossible de capturer une reference live.");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    live_references_ = std::move(references);
}

void ServiceController::set_last_error(const std::string& message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    last_error_ = message;
    state_ = ServiceState::Error;
}

std::optional<CalibrationData> ServiceController::find_override(int camera_id) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    const auto it = calibration_overrides_.find(camera_id);
    if (it == calibration_overrides_.end())
    {
        return std::nullopt;
    }

    return it->second;
}

std::filesystem::path ServiceController::calibration_path_for_camera(int camera_id) const
{
    for (const auto& camera : config_.cameras)
    {
        if (camera.camera_id == camera_id)
        {
            return camera.calibration_path;
        }
    }

    return std::filesystem::path("config") / ("calibration-camera-" + std::to_string(camera_id) + ".json");
}
} // namespace visiondarts
