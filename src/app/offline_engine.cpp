#include "visiondarts/app/offline_engine.hpp"

#include <chrono>
#include <fstream>

#include <opencv2/imgcodecs.hpp>

#include "visiondarts/core/board_model.hpp"
#include "visiondarts/core/json_utils.hpp"
#include "visiondarts/vision/calibration.hpp"

namespace visiondarts
{
namespace
{
void save_debug_outputs(
    const ReplayScenario& scenario,
    const ImpactDetectionArtifacts& artifacts,
    const OfflineRunOptions& options)
{
    const std::filesystem::path output_dir = options.debug_output_root / scenario.scenario_config.name;
    std::filesystem::create_directories(output_dir);

    cv::imwrite((output_dir / "reference.png").string(), scenario.reference_image);
    cv::imwrite((output_dir / "snapshot.png").string(), scenario.snapshot_image);
    cv::imwrite((output_dir / "gray_reference.png").string(), artifacts.gray_reference);
    cv::imwrite((output_dir / "gray_snapshot.png").string(), artifacts.gray_snapshot);
    cv::imwrite((output_dir / "diff.png").string(), artifacts.diff_image);
    cv::imwrite((output_dir / "binary_mask.png").string(), artifacts.binary_mask);
    cv::imwrite((output_dir / "annotated.png").string(), artifacts.annotated_image);

    std::ofstream log_file(output_dir / "debug_log.txt");
    log_file << "scenario=" << scenario.scenario_config.name << '\n';
    log_file << "selected_contour_index=" << artifacts.selected_contour_index << '\n';
    log_file << "reason=" << artifacts.reason << '\n';
    log_file << "tip_x=" << artifacts.tip_image.x << '\n';
    log_file << "tip_y=" << artifacts.tip_image.y << '\n';
    log_file << "contours=" << artifacts.contours.size() << '\n';
}
} // namespace

OfflineVisionEngine::OfflineVisionEngine(PipelineConfig pipeline)
    : pipeline_(std::move(pipeline))
{
}

OfflineRunResult OfflineVisionEngine::run_scenario(const ReplayScenario& scenario, const OfflineRunOptions& options) const
{
    OfflineRunResult run_result;
    run_result.scenario_directory = scenario.directory;
    run_result.scenario_name = scenario.scenario_config.name;

    const auto start_time = std::chrono::steady_clock::now();

    CalibrationData calibration;
    try
    {
        calibration = options.calibration_override.has_value()
            ? *options.calibration_override
            : CalibrationStore::load(scenario.calibration_path);
    }
    catch (const std::exception&)
    {
        CalibrationRequired required;
        required.timestamp_ms = unix_timestamp_ms();
        required.missing_camera_ids = {scenario.scenario_config.camera_id};
        run_result.actual_event = required;
        run_result.matches_expected = scenario.expected_json.empty()
            || json_contains_expected_subset(run_result.actual_event, scenario.expected_json, 1e-3, &run_result.mismatch_path);
        return run_result;
    }

    const ImpactDetectionResult detection = detector_.detect(
        scenario.reference_image,
        scenario.snapshot_image,
        scenario.scenario_config,
        pipeline_,
        calibration);

    if (options.save_debug || scenario.scenario_config.save_debug_images)
    {
        save_debug_outputs(scenario, detection.artifacts, options);
    }

    const ImpactFinal fused = fusion_.fuse(
        {detection.impact},
        options.allow_single_source,
        pipeline_.outlier_threshold);

    ShotResult shot;
    shot.shot_id = make_shot_id(options.shot_sequence);
    shot.timestamp_ms = unix_timestamp_ms();
    shot.processing_ms = static_cast<int>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count());
    shot.cameras_expected = 1;
    shot.camera_impacts = fused.camera_impacts;
    shot.cameras_used = 0;
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
        run_result.actual_event = shot;
    }
    else
    {
        ScoreEngine score_engine;
        shot = score_engine.score_point(fused.point_cible);
        shot.event = shot.status == ShotStatus::Valid ? "shot_detected" : "shot_invalid";
        shot.shot_id = make_shot_id(options.shot_sequence);
        shot.timestamp_ms = unix_timestamp_ms();
        shot.processing_ms = static_cast<int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time).count());
        shot.board_point = fused.point_cible;
        shot.confidence = fused.confidence;
        shot.cameras_expected = 1;
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

        run_result.actual_event = shot;
    }

    if (options.compare_expected && !scenario.expected_json.empty())
    {
        run_result.matches_expected = json_contains_expected_subset(
            run_result.actual_event,
            scenario.expected_json,
            1e-3,
            &run_result.mismatch_path);
    }

    return run_result;
}
} // namespace visiondarts

