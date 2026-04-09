#include "visiondarts/core/config.hpp"

#include <fstream>
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
} // namespace

void from_json(const nlohmann::json& j, ExecutionConfig& config)
{
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
}

void from_json(const nlohmann::json& j, MaskCircle& mask)
{
    mask.center_x = j.at("center_x").get<int>();
    mask.center_y = j.at("center_y").get<int>();
    mask.radius_px = j.at("radius_px").get<int>();
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
