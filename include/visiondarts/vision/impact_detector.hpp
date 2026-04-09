#pragma once

#include <string>
#include <vector>

#include <opencv2/core.hpp>

#include "visiondarts/core/config.hpp"
#include "visiondarts/core/types.hpp"
#include "visiondarts/vision/calibration.hpp"

namespace visiondarts
{
struct ImpactDetectionArtifacts
{
    cv::Mat gray_reference;
    cv::Mat gray_snapshot;
    cv::Mat diff_image;
    cv::Mat binary_mask;
    cv::Mat annotated_image;
    std::vector<std::vector<cv::Point>> contours;
    int selected_contour_index = -1;
    cv::Point2d tip_image{};
    std::string reason;
};

struct ImpactDetectionResult
{
    ImpactCamera impact{};
    ImpactDetectionArtifacts artifacts{};
};

class ImpactDetector
{
  public:
    ImpactDetectionResult detect(
        const cv::Mat& reference,
        const cv::Mat& snapshot,
        const ScenarioConfig& scenario,
        const PipelineConfig& pipeline,
        const CalibrationData& calibration) const;
};
} // namespace visiondarts

