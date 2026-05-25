#include "visiondarts/app/live_engine.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>
#include <utility>

#include <opencv2/imgproc.hpp>

#include "visiondarts/core/board_model.hpp"
#include "visiondarts/core/json_utils.hpp"

namespace visiondarts
{
namespace
{
std::int64_t elapsed_ms(std::chrono::steady_clock::time_point start_time)
{
    return static_cast<std::int64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count());
}
} // namespace

LiveVisionEngine::LiveVisionEngine(PipelineConfig pipeline)
    : pipeline_(std::move(pipeline))
{
}

nlohmann::json LiveVisionEngine::process_shot(
    const std::unordered_map<int, cv::Mat>& references,
    const std::vector<LiveCameraFrame>& snapshots,
    const std::unordered_map<int, CalibrationData>& calibrations,
    const LiveRunOptions& options) const
{
    const auto start_time = std::chrono::steady_clock::now();
    std::vector<ImpactCamera> impacts;

    for (const auto& frame : snapshots)
    {
        ImpactCamera missing_impact;
        missing_impact.camera_id = frame.config.camera_id;

        const auto reference_it = references.find(frame.config.camera_id);
        if (reference_it == references.end() || reference_it->second.empty())
        {
            missing_impact.valid = false;
            missing_impact.reason = "missing_reference";
            impacts.push_back(missing_impact);
            continue;
        }

        const auto calibration_it = calibrations.find(frame.config.camera_id);
        if (calibration_it == calibrations.end() || !calibration_it->second.is_valid())
        {
            missing_impact.valid = false;
            missing_impact.reason = "missing_calibration";
            impacts.push_back(missing_impact);
            continue;
        }

        const ImpactDetectionResult detection = detector_.detect(
            reference_it->second,
            frame.image,
            make_scenario_config(frame),
            pipeline_,
            calibration_it->second);
        impacts.push_back(detection.impact);
    }

    const ImpactFinal fused = fusion_.fuse(
        impacts,
        options.allow_single_source,
        pipeline_.outlier_threshold);

    ShotResult shot;
    shot.shot_id = make_shot_id(options.shot_sequence);
    shot.timestamp_ms = unix_timestamp_ms();
    shot.processing_ms = static_cast<int>(elapsed_ms(start_time));
    shot.cameras_expected = static_cast<int>(snapshots.size());
    shot.cameras_used = 0;
    shot.camera_impacts = fused.camera_impacts;

    for (const auto& impact : fused.camera_impacts)
    {
        if (impact.used_in_fusion)
        {
            ++shot.cameras_used;
        }
    }

    if (!fused.valid)
    {
        shot.event = "shot_invalid";
        shot.status = ShotStatus::Invalid;
        shot.reason = fused.reason;
        shot.confidence = fused.confidence;
        return nlohmann::json(shot);
    }

    ScoreEngine score_engine;
    shot = score_engine.score_point(fused.point_cible);
    shot.event = shot.status == ShotStatus::Valid ? "shot_detected" : "shot_invalid";
    shot.shot_id = make_shot_id(options.shot_sequence);
    shot.timestamp_ms = unix_timestamp_ms();
    shot.processing_ms = static_cast<int>(elapsed_ms(start_time));
    shot.board_point = fused.point_cible;
    shot.confidence = fused.confidence;
    shot.cameras_expected = static_cast<int>(snapshots.size());
    shot.cameras_used = 0;
    shot.camera_impacts = fused.camera_impacts;
    for (const auto& impact : fused.camera_impacts)
    {
        if (impact.used_in_fusion)
        {
            ++shot.cameras_used;
        }
    }
    if (shot.status != ShotStatus::Valid)
    {
        shot.event = "shot_invalid";
        shot.reason = "impact_outside_board";
    }

    return nlohmann::json(shot);
}

double LiveVisionEngine::frame_change_score(
    const std::unordered_map<int, cv::Mat>& references,
    const std::vector<LiveCameraFrame>& frames) const
{
    double max_change = 0.0;
    for (const auto& frame : frames)
    {
        const auto reference_it = references.find(frame.config.camera_id);
        if (reference_it == references.end() || reference_it->second.empty() || frame.image.empty())
        {
            continue;
        }
        if (reference_it->second.size() != frame.image.size())
        {
            continue;
        }

        cv::Mat reference_gray;
        cv::Mat frame_gray;
        cv::Mat diff;
        cv::cvtColor(reference_it->second, reference_gray, cv::COLOR_BGR2GRAY);
        cv::cvtColor(frame.image, frame_gray, cv::COLOR_BGR2GRAY);
        cv::absdiff(reference_gray, frame_gray, diff);
        max_change = std::max(max_change, cv::mean(diff)[0]);
    }
    return max_change;
}

ScenarioConfig LiveVisionEngine::make_scenario_config(const LiveCameraFrame& frame) const
{
    ScenarioConfig scenario;
    scenario.name = "live-camera-" + std::to_string(frame.config.camera_id);
    scenario.camera_id = frame.config.camera_id;
    if (frame.config.mask.has_value())
    {
        scenario.mask = *frame.config.mask;
        return scenario;
    }

    const int width = frame.image.cols;
    const int height = frame.image.rows;
    scenario.mask.center_x = width / 2;
    scenario.mask.center_y = height / 2;
    scenario.mask.radius_px = std::max(1, (std::min(width, height) / 2) - 8);
    return scenario;
}
} // namespace visiondarts
