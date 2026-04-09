#pragma once

#include <filesystem>
#include <vector>

#include <nlohmann/json.hpp>
#include <opencv2/core.hpp>

#include "visiondarts/core/config.hpp"

namespace visiondarts
{
struct ReplayScenario
{
    std::filesystem::path directory;
    std::filesystem::path reference_path;
    std::filesystem::path snapshot_path;
    std::filesystem::path calibration_path;
    std::filesystem::path scenario_path;
    std::filesystem::path expected_path;
    ScenarioConfig scenario_config{};
    bool has_expected_json = false;
    nlohmann::json expected_json;
    cv::Mat reference_image;
    cv::Mat snapshot_image;
};

class IFrameSource
{
  public:
    virtual ~IFrameSource() = default;
    virtual ReplayScenario load_scenario(const std::filesystem::path& scenario_directory) const = 0;
    virtual std::vector<std::filesystem::path> list_scenarios(const std::filesystem::path& path) const = 0;
};

class ReplayFrameSource final : public IFrameSource
{
  public:
    ReplayScenario load_scenario(const std::filesystem::path& scenario_directory) const override;
    std::vector<std::filesystem::path> list_scenarios(const std::filesystem::path& path) const override;
};
} // namespace visiondarts
