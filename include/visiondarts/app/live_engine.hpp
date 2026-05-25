#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <nlohmann/json.hpp>
#include <opencv2/core.hpp>

#include "visiondarts/core/config.hpp"
#include "visiondarts/vision/calibration.hpp"
#include "visiondarts/vision/fusion_engine.hpp"
#include "visiondarts/vision/impact_detector.hpp"
#include "visiondarts/vision/live_camera_source.hpp"

namespace visiondarts
{
struct LiveRunOptions
{
    bool allow_single_source = true;
    std::uint64_t shot_sequence = 1;
};

class LiveVisionEngine
{
  public:
    explicit LiveVisionEngine(PipelineConfig pipeline);

    nlohmann::json process_shot(
        const std::unordered_map<int, cv::Mat>& references,
        const std::vector<LiveCameraFrame>& snapshots,
        const std::unordered_map<int, CalibrationData>& calibrations,
        const LiveRunOptions& options) const;

    double frame_change_score(
        const std::unordered_map<int, cv::Mat>& references,
        const std::vector<LiveCameraFrame>& frames) const;

  private:
    ScenarioConfig make_scenario_config(const LiveCameraFrame& frame) const;

    PipelineConfig pipeline_;
    ImpactDetector detector_{};
    FusionEngine fusion_{};
};
} // namespace visiondarts
