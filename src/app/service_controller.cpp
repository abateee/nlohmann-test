#include "visiondarts/app/service_controller.hpp"

#include "visiondarts/core/json_utils.hpp"
#include "visiondarts/vision/calibration.hpp"

namespace visiondarts
{
ServiceController::ServiceController(AppConfig config)
    : config_(std::move(config))
    , engine_(config_.pipeline)
    , publisher_(config_.backend)
{
    publisher_.set_error_callback([this](const std::string& message) {
        set_last_error(message);
    });
    publisher_.start();
}

ServiceController::~ServiceController()
{
    stop();
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
    return nlohmann::json{{"accepted", true}, {"state", to_string(state_)}};
}

nlohmann::json ServiceController::reset_reference()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return nlohmann::json{
        {"accepted", true},
        {"message", "La reference sera rechargee au prochain traitement de scenario."},
    };
}

nlohmann::json ServiceController::healthcheck() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return nlohmann::json{
        {"service", "vision"},
        {"status", last_error_.has_value() ? "degraded" : "ok"},
        {"running", state_ == ServiceState::Running},
        {"state", to_string(state_)},
        {"cameras_configured", 1},
        {"cameras_opened", 1},
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
                calibration_loaded_ = result.actual_event.value("event", std::string{}) != "calibration_required";
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
} // namespace visiondarts
