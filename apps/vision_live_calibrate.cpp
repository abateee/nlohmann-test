#include <algorithm>
#include <exception>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <opencv2/imgcodecs.hpp>

#include "visiondarts/core/config.hpp"
#include "visiondarts/vision/calibration.hpp"
#include "visiondarts/vision/live_camera_source.hpp"

namespace
{
constexpr int kRequiredPoints = 4;

const std::vector<cv::Point2d> kBoardPoints = {
    {0.0, 1.0},
    {1.0, 0.0},
    {0.0, -1.0},
    {-1.0, 0.0},
};

const std::vector<std::string> kPointLabels = {
    "haut double",
    "droite double",
    "bas double",
    "gauche double",
};

std::vector<visiondarts::LiveCameraConfig> enabled_cameras(const visiondarts::AppConfig& config)
{
    std::vector<visiondarts::LiveCameraConfig> cameras;
    if (config.cameras.empty())
    {
        cameras.push_back(visiondarts::LiveCameraConfig{});
        return cameras;
    }

    for (const auto& camera : config.cameras)
    {
        if (camera.enabled)
        {
            cameras.push_back(camera);
        }
    }
    return cameras;
}

visiondarts::LiveCameraConfig camera_by_id(
    const std::vector<visiondarts::LiveCameraConfig>& cameras,
    int camera_id)
{
    const auto it = std::find_if(cameras.begin(), cameras.end(), [camera_id](const auto& camera) {
        return camera.camera_id == camera_id;
    });
    if (it == cameras.end())
    {
        throw std::runtime_error("camera_id introuvable dans la config: " + std::to_string(camera_id));
    }
    return *it;
}

std::vector<visiondarts::LiveCameraConfig> select_cameras(
    const visiondarts::AppConfig& config,
    int argc,
    char** argv)
{
    const auto cameras = enabled_cameras(config);
    if (cameras.empty())
    {
        throw std::runtime_error("Aucune camera active dans la config.");
    }

    if (argc >= 3 && std::string(argv[2]) == "--all")
    {
        return cameras;
    }

    if (argc >= 3)
    {
        return {camera_by_id(cameras, std::stoi(argv[2]))};
    }

    if (cameras.size() == 1)
    {
        return cameras;
    }

    throw std::runtime_error("Plusieurs cameras sont configurees. Fournis un camera_id ou --all.");
}

std::filesystem::path capture_reference_image(const visiondarts::LiveCameraConfig& camera)
{
    visiondarts::LiveCameraSource source;
    source.open({camera});

    std::vector<visiondarts::LiveCameraFrame> frames;
    for (int attempt = 0; attempt < 20; ++attempt)
    {
        frames = source.capture_frames();
        if (!frames.empty() && !frames.front().image.empty())
        {
            break;
        }
    }

    source.close();

    if (frames.empty() || frames.front().image.empty())
    {
        throw std::runtime_error("Impossible de capturer une image pour la camera " + std::to_string(camera.camera_id));
    }

    const std::filesystem::path output_path =
        std::filesystem::path("build") / "calibration" / ("camera-" + std::to_string(camera.camera_id) + "-calibration-frame.png");
    std::filesystem::create_directories(output_path.parent_path());

    if (!cv::imwrite(output_path.string(), frames.front().image))
    {
        throw std::runtime_error("Impossible d'ecrire l'image de calibration: " + output_path.string());
    }

    return output_path;
}

cv::Point2d read_point(const std::string& label)
{
    for (;;)
    {
        std::cout << "Point " << label << " (format: x y): ";
        std::string line;
        if (!std::getline(std::cin, line))
        {
            throw std::runtime_error("Lecture terminal interrompue.");
        }

        std::istringstream stream(line);
        double x = 0.0;
        double y = 0.0;
        if (stream >> x >> y)
        {
            return {x, y};
        }

        std::cout << "Format invalide. Exemple: 640 120\n";
    }
}

bool calibrate_camera(const visiondarts::LiveCameraConfig& camera)
{
    const auto image_path = capture_reference_image(camera);

    std::cout << "\nCalibration camera " << camera.camera_id << " (device_index=" << camera.device_index << ")\n";
    std::cout << "Image capturee: " << image_path.string() << '\n';
    std::cout << "Ouvre cette image, lis les coordonnees pixel, puis saisis les 4 points.\n";
    std::cout << "Ordre requis: haut double, droite double, bas double, gauche double.\n";

    std::vector<cv::Point2d> image_points;
    image_points.reserve(kRequiredPoints);
    for (const auto& label : kPointLabels)
    {
        image_points.push_back(read_point(label));
    }

    const auto calibration = visiondarts::compute_calibration(
        camera.camera_id,
        image_points,
        kBoardPoints,
        0.0);
    visiondarts::CalibrationStore::save_json(camera.calibration_path, calibration);

    std::cout << "Calibration sauvegardee: " << camera.calibration_path.string() << "\n";
    return true;
}

void print_usage()
{
    std::cout << "Usage:\n"
              << "  vision_live_calibrate.exe config/live_windows.json\n"
              << "  vision_live_calibrate.exe config/live_windows.json <camera_id>\n"
              << "  vision_live_calibrate.exe config/live_windows.json --all\n";
}
} // namespace

int main(int argc, char** argv)
{
    try
    {
        if (argc < 2)
        {
            print_usage();
            return 1;
        }

        const auto config = visiondarts::load_app_config(argv[1]);
        const auto cameras = select_cameras(config, argc, argv);

        bool all_saved = true;
        for (const auto& camera : cameras)
        {
            all_saved = calibrate_camera(camera) && all_saved;
        }

        return all_saved ? 0 : 2;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Erreur: " << exception.what() << '\n';
        return 1;
    }
}
