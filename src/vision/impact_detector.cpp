#include "visiondarts/vision/impact_detector.hpp"

#include <algorithm>
#include <cmath>

#include <opencv2/imgproc.hpp>

namespace visiondarts
{
namespace
{
double clamp01(double value)
{
    return std::max(0.0, std::min(1.0, value));
}

double distance_between(const cv::Point2d& a, const cv::Point2d& b)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return std::sqrt((dx * dx) + (dy * dy));
}
} // namespace

ImpactDetectionResult ImpactDetector::detect(
    const cv::Mat& reference,
    const cv::Mat& snapshot,
    const ScenarioConfig& scenario,
    const PipelineConfig& pipeline,
    const CalibrationData& calibration) const
{
    ImpactDetectionResult result;
    result.impact.camera_id = scenario.camera_id;

    if (reference.empty() || snapshot.empty())
    {
        result.impact.valid = false;
        result.impact.reason = "empty_input_image";
        result.artifacts.reason = result.impact.reason;
        return result;
    }

    if (reference.size() != snapshot.size())
    {
        result.impact.valid = false;
        result.impact.reason = "image_size_mismatch";
        result.artifacts.reason = result.impact.reason;
        return result;
    }

    cv::cvtColor(reference, result.artifacts.gray_reference, cv::COLOR_BGR2GRAY);
    cv::cvtColor(snapshot, result.artifacts.gray_snapshot, cv::COLOR_BGR2GRAY);

    const int blur_kernel = std::max(3, pipeline.blur_kernel_size | 1);
    cv::GaussianBlur(result.artifacts.gray_reference, result.artifacts.gray_reference, cv::Size(blur_kernel, blur_kernel), 0.0);
    cv::GaussianBlur(result.artifacts.gray_snapshot, result.artifacts.gray_snapshot, cv::Size(blur_kernel, blur_kernel), 0.0);

    cv::absdiff(result.artifacts.gray_reference, result.artifacts.gray_snapshot, result.artifacts.diff_image);

    const int diff_threshold = scenario.diff_threshold_override.has_value()
        ? static_cast<int>(*scenario.diff_threshold_override)
        : pipeline.diff_threshold;
    cv::threshold(result.artifacts.diff_image, result.artifacts.binary_mask, diff_threshold, 255, cv::THRESH_BINARY);

    const int morph_kernel_size = std::max(1, pipeline.morph_kernel_size);
    const cv::Mat morph_kernel = cv::getStructuringElement(
        cv::MORPH_ELLIPSE,
        cv::Size((morph_kernel_size * 2) + 1, (morph_kernel_size * 2) + 1));
    cv::morphologyEx(result.artifacts.binary_mask, result.artifacts.binary_mask, cv::MORPH_OPEN, morph_kernel);
    cv::morphologyEx(result.artifacts.binary_mask, result.artifacts.binary_mask, cv::MORPH_CLOSE, morph_kernel);

    cv::Mat board_mask(result.artifacts.binary_mask.size(), CV_8UC1, cv::Scalar(0));
    cv::circle(board_mask, cv::Point(scenario.mask.center_x, scenario.mask.center_y), scenario.mask.radius_px, cv::Scalar(255), cv::FILLED);
    cv::bitwise_and(result.artifacts.binary_mask, board_mask, result.artifacts.binary_mask);

    cv::findContours(result.artifacts.binary_mask.clone(), result.artifacts.contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    const double min_contour_area = scenario.min_contour_area_override.value_or(pipeline.min_contour_area);
    const cv::Point2d board_center(static_cast<double>(scenario.mask.center_x), static_cast<double>(scenario.mask.center_y));
    const double board_radius = static_cast<double>(scenario.mask.radius_px);

    double best_score = -1.0;
    double second_best_score = -1.0;
    std::size_t best_index = 0;
    cv::Point2d best_tip;
    cv::Point2d second_best_tip;
    for (std::size_t index = 0; index < result.artifacts.contours.size(); ++index)
    {
        const auto& contour = result.artifacts.contours.at(index);
        const double area = cv::contourArea(contour);
        if (area < min_contour_area || area > pipeline.max_contour_area)
        {
            continue;
        }

        cv::Point2d tip = contour.front();
        double best_tip_distance = distance_between(tip, board_center);
        for (const auto& point : contour)
        {
            const double candidate_distance = distance_between(point, board_center);
            if (candidate_distance < best_tip_distance)
            {
                tip = point;
                best_tip_distance = candidate_distance;
            }
        }

        const cv::Rect bounds = cv::boundingRect(contour);
        const double length_score = clamp01(static_cast<double>(std::max(bounds.width, bounds.height)) / 90.0);
        const double area_score = clamp01(area / 500.0);
        const double min_side = static_cast<double>(std::max(1, std::min(bounds.width, bounds.height)));
        const double max_side = static_cast<double>(std::max(bounds.width, bounds.height));
        const double elongation = max_side / min_side;
        const double elongation_score = clamp01((elongation - 1.0) / 4.0);
        const double center_bonus = clamp01(1.0 - (best_tip_distance / board_radius));
        const double rim_bonus = clamp01(1.0 - (std::abs(best_tip_distance - board_radius) / (board_radius * 0.20)));
        const double placement_score = std::max(center_bonus, rim_bonus);
        const double score = (0.25 * area_score) + (0.30 * length_score) + (0.20 * elongation_score) + (0.25 * placement_score);

        if (score > best_score)
        {
            second_best_score = best_score;
            second_best_tip = best_tip;
            best_score = score;
            best_index = index;
            best_tip = tip;
        }
        else if (score > second_best_score)
        {
            second_best_score = score;
            second_best_tip = tip;
        }
    }

    result.artifacts.annotated_image = snapshot.clone();
    if (best_score < pipeline.quality_floor)
    {
        result.impact.valid = false;
        result.impact.reason = "no_plausible_contour";
        result.artifacts.reason = result.impact.reason;
        return result;
    }

    if (second_best_score >= (best_score * 0.92) && distance_between(best_tip, second_best_tip) >= 20.0)
    {
        result.impact.valid = false;
        result.impact.reason = "ambiguous_contour";
        result.artifacts.reason = result.impact.reason;
        return result;
    }

    result.artifacts.selected_contour_index = static_cast<int>(best_index);
    result.artifacts.tip_image = best_tip;
    cv::drawContours(result.artifacts.annotated_image, result.artifacts.contours, static_cast<int>(best_index), cv::Scalar(0, 0, 255), 2);
    cv::circle(result.artifacts.annotated_image, best_tip, 6, cv::Scalar(0, 255, 0), cv::FILLED);

    result.impact.point_cible = project_image_point(calibration, best_tip);
    result.impact.quality = clamp01(best_score);
    result.impact.valid = true;
    result.impact.used_in_fusion = false;
    result.artifacts.reason = "ok";
    return result;
}
} // namespace visiondarts
