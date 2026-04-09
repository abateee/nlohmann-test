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
    std::string post_url = "http://127.0.0.1:3000/api/game301/joueurs";
    std::string service_host = "127.0.0.1";
    int service_port = 3000;
    int post_timeout_ms = 500;
    int post_retry_count = 3;
};

struct AppConfig
{
    ExecutionConfig execution{};
    PipelineConfig pipeline{};
    BackendConfig backend{};
};

struct MaskCircle
{
    int center_x = 0;
    int center_y = 0;
    int radius_px = 0;
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
void from_json(const nlohmann::json& j, AppConfig& config);
void from_json(const nlohmann::json& j, MaskCircle& mask);
void from_json(const nlohmann::json& j, ScenarioConfig& config);

AppConfig load_app_config(const std::filesystem::path& path);
ScenarioConfig load_scenario_config(const std::filesystem::path& path);
} // namespace visiondarts

