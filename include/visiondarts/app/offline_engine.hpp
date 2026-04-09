#pragma once

#include <filesystem>
#include <optional>

#include <nlohmann/json.hpp>

#include "visiondarts/core/config.hpp"
#include "visiondarts/vision/fusion_engine.hpp"
#include "visiondarts/vision/impact_detector.hpp"
#include "visiondarts/vision/replay.hpp"

namespace visiondarts
{
struct OfflineRunOptions
{
    bool compare_expected = true;
    bool save_debug = false;
    std::filesystem::path debug_output_root = "build/debug_output";
    bool allow_single_source = true;
    std::optional<CalibrationData> calibration_override;
    std::uint64_t shot_sequence = 1;
};

struct OfflineRunResult
{
    std::filesystem::path scenario_directory;
    std::string scenario_name;
    nlohmann::json actual_event;
    bool matches_expected = true;
    std::string mismatch_path;
};

class OfflineVisionEngine
{
  public:
    explicit OfflineVisionEngine(PipelineConfig pipeline);

    OfflineRunResult run_scenario(const ReplayScenario& scenario, const OfflineRunOptions& options) const;

  private:
    PipelineConfig pipeline_;
    ImpactDetector detector_{};
    FusionEngine fusion_{};
};
} // namespace visiondarts

