#pragma once

#include <filesystem>
#include <stdexcept>
#include <vector>

#include <opencv2/core.hpp>

#include "visiondarts/core/types.hpp"

namespace visiondarts
{
class CalibrationMissingError : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

class CalibrationLoadError : public std::runtime_error
{
  public:
    using std::runtime_error::runtime_error;
};

struct CalibrationData
{
    int camera_id = 0;
    cv::Mat homography;
    double offset_angle_deg = 0.0;
    std::vector<cv::Point2d> points_image;
    std::vector<cv::Point2d> points_board;

    bool is_valid() const;
};

class CalibrationStore
{
  public:
    static CalibrationData load(const std::filesystem::path& path);
    static void save_file_storage_json(const std::filesystem::path& path, const CalibrationData& calibration);
};

CalibrationData compute_calibration(
    int camera_id,
    const std::vector<cv::Point2d>& points_image,
    const std::vector<cv::Point2d>& points_board,
    double offset_angle_deg);

BoardPoint project_image_point(const CalibrationData& calibration, const cv::Point2d& image_point);
cv::Point2d project_board_point_to_image(const CalibrationData& calibration, const BoardPoint& board_point);
} // namespace visiondarts
