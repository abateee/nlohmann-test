#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace visiondarts
{
struct ExecutionConfig
{
    std::string mode = "replay";
    std::filesystem::path scenario_root = "fixtures";
    bool allow_single_source = true;
    bool debug_save_intermediates = false;
    std::filesystem::path debug_output_root = "build/debug_output";
    bool run_all_on_start = true;
};

struct PipelineConfig
{
    int diff_threshold = 30;
    int blur_kernel_size = 5;
    int morph_kernel_size = 3;
    double min_contour_area = 40.0;
    double max_contour_area = 100000.0;
    double outlier_threshold = 0.08;
    double quality_floor = 0.20;
};

struct BackendConfig
{
    std::string post_url = "http://127.0.0.1:8080/vision/events";
    std::string service_host = "127.0.0.1";
    int service_port = 8090;
    int post_timeout_ms = 500;
    int post_retry_count = 3;
};

struct MaskCircle
{
    int center_x = 0;
    int center_y = 0;
    int radius_px = 0;
};

struct LiveCameraConfig
{
    int camera_id = 1;
    int device_index = 0;
    int width = 0;
    int height = 0;
    int fps = 0;
    std::filesystem::path calibration_path = "config/calibration-camera-1.json";
    bool enabled = true;
    std::optional<MaskCircle> mask;
};

struct LiveConfig
{
    int reference_stability_frames = 5;
    double shot_change_threshold = 8.0;
    int stabilization_frames = 5;
    int loop_sleep_ms = 50;
    int min_ms_between_shots = 750;
};

struct AppConfig
{
    ExecutionConfig execution{};
    PipelineConfig pipeline{};
    BackendConfig backend{};
    LiveConfig live{};
    std::vector<LiveCameraConfig> cameras{};
};

struct ScenarioConfig
{
    std::string name;
    int camera_id = 1;
    MaskCircle mask{};
    bool save_debug_images = false;
    std::optional<double> diff_threshold_override;
    std::optional<double> min_contour_area_override;
};

void from_json(const nlohmann::json& j, ExecutionConfig& config);
void from_json(const nlohmann::json& j, PipelineConfig& config);
void from_json(const nlohmann::json& j, BackendConfig& config);
void from_json(const nlohmann::json& j, LiveCameraConfig& config);
void from_json(const nlohmann::json& j, LiveConfig& config);
void from_json(const nlohmann::json& j, AppConfig& config);
void from_json(const nlohmann::json& j, MaskCircle& mask);
void from_json(const nlohmann::json& j, ScenarioConfig& config);

AppConfig load_app_config(const std::filesystem::path& path);
ScenarioConfig load_scenario_config(const std::filesystem::path& path);
} // namespace visiondarts
