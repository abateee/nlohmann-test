#include "visiondarts/core/config.hpp"

#include <fstream>
#include <set>
#include <stdexcept>

namespace visiondarts
{
namespace
{
nlohmann::json load_json_file(const std::filesystem::path& path)
{
    std::ifstream input(path);
    if (!input)
    {
        throw std::runtime_error("Impossible d'ouvrir le fichier JSON: " + path.string());
    }

    return nlohmann::json::parse(input);
}

int enabled_camera_count(const std::vector<LiveCameraConfig>& cameras)
{
    int count = 0;
    for (const auto& camera : cameras)
    {
        if (camera.enabled)
        {
            ++count;
        }
    }
    return count;
}

bool is_valid_capture_backend(const std::string& capture_backend)
{
    return capture_backend == "auto" || capture_backend == "dshow" || capture_backend == "msmf";
}

void validate_live_config(const AppConfig& config)
{
    if (config.execution.mode != "live")
    {
        return;
    }

    const int enabled_count = enabled_camera_count(config.cameras);
    if (enabled_count < 1)
    {
        throw std::runtime_error("La config live doit activer au moins une camera.");
    }
    if (enabled_count > 3)
    {
        throw std::runtime_error("La config live supporte au maximum 3 cameras actives.");
    }

    std::set<int> camera_ids;
    std::set<int> device_indexes;
    for (const auto& camera : config.cameras)
    {
        if (!camera.enabled)
        {
            continue;
        }
        if (camera.camera_id <= 0)
        {
            throw std::runtime_error("camera_id live doit etre strictement positif.");
        }
        if (camera.device_index < 0)
        {
            throw std::runtime_error("device_index live doit etre positif ou nul.");
        }
        if (!camera_ids.insert(camera.camera_id).second)
        {
            throw std::runtime_error("camera_id live duplique: " + std::to_string(camera.camera_id));
        }
        if (!device_indexes.insert(camera.device_index).second)
        {
            throw std::runtime_error("device_index live duplique: " + std::to_string(camera.device_index));
        }
        if (camera.calibration_path.empty())
        {
            throw std::runtime_error("calibration_path live manquant pour camera_id=" + std::to_string(camera.camera_id));
        }
        if (!is_valid_capture_backend(camera.capture_backend))
        {
            throw std::runtime_error("capture_backend live invalide pour camera_id=" + std::to_string(camera.camera_id));
        }
    }
}
} // namespace

void from_json(const nlohmann::json& j, ExecutionConfig& config)
{
    config.mode = j.value("mode", std::string{"replay"});
    config.scenario_root = j.value("scenario_root", std::string{"fixtures"});
    config.allow_single_source = j.value("allow_single_source", true);
    config.debug_save_intermediates = j.value("debug_save_intermediates", false);
    config.debug_output_root = j.value("debug_output_root", std::string{"build/debug_output"});
    config.run_all_on_start = j.value("run_all_on_start", true);
}

void from_json(const nlohmann::json& j, PipelineConfig& config)
{
    config.diff_threshold = j.value("diff_threshold", 30);
    config.blur_kernel_size = j.value("blur_kernel_size", 5);
    config.morph_kernel_size = j.value("morph_kernel_size", 3);
    config.min_contour_area = j.value("min_contour_area", 40.0);
    config.max_contour_area = j.value("max_contour_area", 100000.0);
    config.outlier_threshold = j.value("outlier_threshold", 0.08);
    config.quality_floor = j.value("quality_floor", 0.20);
}

void from_json(const nlohmann::json& j, BackendConfig& config)
{
    config.post_url = j.value("post_url", std::string{"http://127.0.0.1:8080/vision/events"});
    config.service_host = j.value("service_host", std::string{"127.0.0.1"});
    config.service_port = j.value("service_port", 8090);
    config.post_timeout_ms = j.value("post_timeout_ms", 500);
    config.post_retry_count = j.value("post_retry_count", 3);
}

void from_json(const nlohmann::json& j, MaskCircle& mask)
{
    mask.center_x = j.at("center_x").get<int>();
    mask.center_y = j.at("center_y").get<int>();
    mask.radius_px = j.at("radius_px").get<int>();
}

void from_json(const nlohmann::json& j, LiveCameraConfig& config)
{
    config.camera_id = j.value("camera_id", 1);
    config.device_index = j.value("device_index", 0);
    config.width = j.value("width", 0);
    config.height = j.value("height", 0);
    config.fps = j.value("fps", 0);
    config.capture_backend = j.value("capture_backend", config.capture_backend);
    config.calibration_path = j.value("calibration_path", std::string{"config/calibration-camera-1.json"});
    config.enabled = j.value("enabled", true);
    if (j.contains("mask"))
    {
        config.mask = j.at("mask").get<MaskCircle>();
    }
}

void from_json(const nlohmann::json& j, LiveConfig& config)
{
    config.reference_stability_frames = j.value("reference_stability_frames", 5);
    config.shot_change_threshold = j.value("shot_change_threshold", 8.0);
    config.stabilization_frames = j.value("stabilization_frames", 5);
    config.loop_sleep_ms = j.value("loop_sleep_ms", 50);
    config.min_ms_between_shots = j.value("min_ms_between_shots", 750);
}

void from_json(const nlohmann::json& j, AppConfig& config)
{
    if (j.contains("execution"))
    {
        config.execution = j.at("execution").get<ExecutionConfig>();
    }
    if (j.contains("pipeline"))
    {
        config.pipeline = j.at("pipeline").get<PipelineConfig>();
    }
    if (j.contains("backend"))
    {
        config.backend = j.at("backend").get<BackendConfig>();
    }
    if (j.contains("live"))
    {
        config.live = j.at("live").get<LiveConfig>();
    }
    if (j.contains("cameras"))
    {
        config.cameras = j.at("cameras").get<std::vector<LiveCameraConfig>>();
    }
    validate_live_config(config);
}

void from_json(const nlohmann::json& j, ScenarioConfig& config)
{
    config.name = j.at("name").get<std::string>();
    config.camera_id = j.value("camera_id", 1);
    config.mask = j.at("mask").get<MaskCircle>();
    config.save_debug_images = j.value("save_debug_images", false);
    if (j.contains("diff_threshold_override"))
    {
        config.diff_threshold_override = j.at("diff_threshold_override").get<double>();
    }
    if (j.contains("min_contour_area_override"))
    {
        config.min_contour_area_override = j.at("min_contour_area_override").get<double>();
    }
}

AppConfig load_app_config(const std::filesystem::path& path)
{
    return load_json_file(path).get<AppConfig>();
}

ScenarioConfig load_scenario_config(const std::filesystem::path& path)
{
    return load_json_file(path).get<ScenarioConfig>();
}
} // namespace visiondarts
