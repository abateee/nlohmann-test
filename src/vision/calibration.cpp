#include "visiondarts/vision/calibration.hpp"

#include <cmath>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core/persistence.hpp>

namespace visiondarts
{
namespace
{
constexpr double kPi = 3.14159265358979323846;

std::vector<cv::Point2d> parse_point_vector(const nlohmann::json& values)
{
    std::vector<cv::Point2d> points;
    points.reserve(values.size());
    for (const auto& item : values)
    {
        points.emplace_back(item.at("x").get<double>(), item.at("y").get<double>());
    }
    return points;
}

BoardPoint rotate_board_point(const BoardPoint& point, double angle_deg)
{
    if (std::abs(angle_deg) < 1e-12)
    {
        return point;
    }

    const double angle_rad = angle_deg * kPi / 180.0;
    const double cos_angle = std::cos(angle_rad);
    const double sin_angle = std::sin(angle_rad);
    return BoardPoint{
        (point.x * cos_angle) - (point.y * sin_angle),
        (point.x * sin_angle) + (point.y * cos_angle),
    };
}
} // namespace

bool CalibrationData::is_valid() const
{
    return !homography.empty() && homography.rows == 3 && homography.cols == 3;
}

CalibrationData CalibrationStore::load(const std::filesystem::path& path)
{
    if (!std::filesystem::exists(path))
    {
        throw CalibrationMissingError("Calibration introuvable: " + path.string());
    }

    if (path.extension() == ".json" && path.filename().string().find(".fs.") == std::string::npos)
    {
        try
        {
            std::ifstream input(path);
            if (!input)
            {
                throw CalibrationLoadError("Impossible d'ouvrir la calibration JSON: " + path.string());
            }

            const nlohmann::json json = nlohmann::json::parse(input);
            const int camera_id = json.value("camera_id", 0);
            const double offset_angle_deg = json.value("offset_angle_deg", 0.0);
            const auto points_image = parse_point_vector(json.at("points_image"));
            const auto points_board = parse_point_vector(json.at("points_board"));
            try
            {
                return compute_calibration(camera_id, points_image, points_board, offset_angle_deg);
            }
            catch (const std::exception& exception)
            {
                throw CalibrationLoadError(exception.what());
            }
        }
        catch (const CalibrationLoadError&)
        {
            throw;
        }
        catch (const std::exception& exception)
        {
            throw CalibrationLoadError(exception.what());
        }
    }

    cv::FileStorage storage(path.string(), cv::FileStorage::READ | cv::FileStorage::FORMAT_JSON);
    if (!storage.isOpened())
    {
        throw CalibrationLoadError("Impossible d'ouvrir la calibration FileStorage: " + path.string());
    }

    CalibrationData calibration;
    storage["camera_id"] >> calibration.camera_id;
    storage["offset_angle_deg"] >> calibration.offset_angle_deg;
    storage["homography"] >> calibration.homography;
    storage["points_image"] >> calibration.points_image;
    storage["points_board"] >> calibration.points_board;
    if (!calibration.is_valid())
    {
        throw CalibrationLoadError("Calibration FileStorage invalide: " + path.string());
    }
    return calibration;
}

void CalibrationStore::save_file_storage_json(const std::filesystem::path& path, const CalibrationData& calibration)
{
    cv::FileStorage storage(path.string(), cv::FileStorage::WRITE | cv::FileStorage::FORMAT_JSON);
    if (!storage.isOpened())
    {
        throw std::runtime_error("Impossible d'ecrire la calibration: " + path.string());
    }

    storage << "camera_id" << calibration.camera_id;
    storage << "offset_angle_deg" << calibration.offset_angle_deg;
    storage << "homography" << calibration.homography;
    storage << "points_image" << calibration.points_image;
    storage << "points_board" << calibration.points_board;
}

CalibrationData compute_calibration(
    int camera_id,
    const std::vector<cv::Point2d>& points_image,
    const std::vector<cv::Point2d>& points_board,
    double offset_angle_deg)
{
    if (points_image.size() < 4 || points_board.size() < 4 || points_image.size() != points_board.size())
    {
        throw std::runtime_error("Une calibration valide demande au moins 4 couples de points coherents.");
    }

    CalibrationData calibration;
    calibration.camera_id = camera_id;
    calibration.offset_angle_deg = offset_angle_deg;
    calibration.points_image = points_image;
    calibration.points_board = points_board;
    calibration.homography = cv::findHomography(points_image, points_board, cv::RANSAC);

    if (calibration.homography.empty())
    {
        throw std::runtime_error("Le calcul d'homographie a echoue.");
    }

    return calibration;
}

BoardPoint project_image_point(const CalibrationData& calibration, const cv::Point2d& image_point)
{
    if (!calibration.is_valid())
    {
        throw std::runtime_error("Projection impossible sans homographie valide.");
    }

    std::vector<cv::Point2d> input_points = {image_point};
    std::vector<cv::Point2d> output_points;
    cv::perspectiveTransform(input_points, output_points, calibration.homography);

    if (output_points.empty())
    {
        throw std::runtime_error("La projection du point image a echoue.");
    }

    return rotate_board_point(BoardPoint{output_points.front().x, output_points.front().y}, calibration.offset_angle_deg);
}

cv::Point2d project_board_point_to_image(const CalibrationData& calibration, const BoardPoint& board_point)
{
    if (!calibration.is_valid())
    {
        throw std::runtime_error("Projection inverse impossible sans homographie valide.");
    }

    cv::Mat inverse_homography;
    cv::invert(calibration.homography, inverse_homography);

    const BoardPoint unrotated_board_point = rotate_board_point(board_point, -calibration.offset_angle_deg);
    std::vector<cv::Point2d> board_points = {cv::Point2d(unrotated_board_point.x, unrotated_board_point.y)};
    std::vector<cv::Point2d> image_points;
    cv::perspectiveTransform(board_points, image_points, inverse_homography);

    if (image_points.empty())
    {
        throw std::runtime_error("La projection inverse du point cible a echoue.");
    }

    return image_points.front();
}
} // namespace visiondarts
