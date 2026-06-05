#include <chrono>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <nlohmann/json.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>

#include "visiondarts/core/config.hpp"
#include "visiondarts/vision/live_camera_source.hpp"

namespace
{
struct CameraProbe
{
    int camera_id = 0;
    int device_index = 0;
    int requested_width = 0;
    int requested_height = 0;
    int requested_fps = 0;
    std::string capture_backend;
    bool configured = false;
    bool opened = false;
    bool frame_captured = false;
    int actual_width = 0;
    int actual_height = 0;
    double actual_fps = 0.0;
    std::filesystem::path capture_path;
    std::string error;
};

void print_usage()
{
    std::cout << "Usage:\n"
              << "  vision_camera_diagnostics.exe\n"
              << "  vision_camera_diagnostics.exe config/live_windows.json\n"
              << "  vision_camera_diagnostics.exe config/live_windows.json --scan 10\n\n"
              << "Options:\n"
              << "  --scan <max_index>    Scanne les device_index de 0 a max_index.\n"
              << "  --output <dir>        Dossier de sortie des captures et du JSON.\n";
}

std::vector<visiondarts::LiveCameraConfig> enabled_cameras(const visiondarts::AppConfig& config)
{
    std::vector<visiondarts::LiveCameraConfig> cameras;
    for (const auto& camera : config.cameras)
    {
        if (camera.enabled)
        {
            cameras.push_back(camera);
        }
    }
    return cameras;
}

CameraProbe probe_camera(
    const visiondarts::LiveCameraConfig& config,
    const std::filesystem::path& output_dir,
    bool configured)
{
    CameraProbe probe;
    probe.camera_id = config.camera_id;
    probe.device_index = config.device_index;
    probe.requested_width = config.width;
    probe.requested_height = config.height;
    probe.requested_fps = config.fps;
    probe.capture_backend = config.capture_backend;
    probe.configured = configured;

    try
    {
        cv::VideoCapture capture;
        if (!capture.open(config.device_index, visiondarts::opencv_capture_backend(config.capture_backend)))
        {
            probe.error = "open_failed";
            return probe;
        }
        probe.opened = true;

        if (config.width > 0)
        {
            capture.set(cv::CAP_PROP_FRAME_WIDTH, config.width);
        }
        if (config.height > 0)
        {
            capture.set(cv::CAP_PROP_FRAME_HEIGHT, config.height);
        }
        if (config.fps > 0)
        {
            capture.set(cv::CAP_PROP_FPS, config.fps);
        }

        cv::Mat frame;
        for (int attempt = 0; attempt < 20; ++attempt)
        {
            if (capture.read(frame) && !frame.empty())
            {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        probe.actual_width = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_WIDTH));
        probe.actual_height = static_cast<int>(capture.get(cv::CAP_PROP_FRAME_HEIGHT));
        probe.actual_fps = capture.get(cv::CAP_PROP_FPS);

        if (frame.empty())
        {
            probe.error = "capture_failed";
            return probe;
        }

        std::filesystem::create_directories(output_dir);
        const std::string file_name = configured
            ? "camera-" + std::to_string(config.camera_id) + "-device-" + std::to_string(config.device_index) + ".png"
            : "scan-device-" + std::to_string(config.device_index) + ".png";
        probe.capture_path = output_dir / file_name;
        probe.frame_captured = cv::imwrite(probe.capture_path.string(), frame);
        if (!probe.frame_captured)
        {
            probe.error = "write_capture_failed";
        }
    }
    catch (const std::exception& exception)
    {
        probe.error = exception.what();
    }

    return probe;
}

nlohmann::json to_json(const CameraProbe& probe)
{
    return {
        {"camera_id", probe.configured ? nlohmann::json(probe.camera_id) : nlohmann::json(nullptr)},
        {"device_index", probe.device_index},
        {"configured", probe.configured},
        {"opened", probe.opened},
        {"frame_captured", probe.frame_captured},
        {"requested_width", probe.requested_width},
        {"requested_height", probe.requested_height},
        {"requested_fps", probe.requested_fps},
        {"capture_backend", probe.capture_backend},
        {"actual_width", probe.actual_width},
        {"actual_height", probe.actual_height},
        {"actual_fps", probe.actual_fps},
        {"capture_path", probe.capture_path.empty() ? nlohmann::json(nullptr) : nlohmann::json(probe.capture_path.string())},
        {"error", probe.error.empty() ? nlohmann::json(nullptr) : nlohmann::json(probe.error)},
    };
}

void print_probe(const CameraProbe& probe)
{
    std::cout << "- device_index=" << probe.device_index;
    if (probe.configured)
    {
        std::cout << " camera_id=" << probe.camera_id;
    }
    std::cout << " backend=" << probe.capture_backend;
    std::cout << " opened=" << (probe.opened ? "yes" : "no")
              << " frame=" << (probe.frame_captured ? "yes" : "no");
    if (probe.actual_width > 0 && probe.actual_height > 0)
    {
        std::cout << " actual=" << probe.actual_width << "x" << probe.actual_height
                  << " fps=" << probe.actual_fps;
    }
    if (!probe.capture_path.empty())
    {
        std::cout << " capture=" << probe.capture_path.string();
    }
    if (!probe.error.empty())
    {
        std::cout << " error=" << probe.error;
    }
    std::cout << '\n';
}
} // namespace

int main(int argc, char** argv)
{
    try
    {
        std::filesystem::path config_path;
        std::filesystem::path output_dir = std::filesystem::path("build") / "camera_diagnostics";
        int scan_max_index = -1;

        for (int index = 1; index < argc; ++index)
        {
            const std::string arg = argv[index];
            if (arg == "--help" || arg == "-h")
            {
                print_usage();
                return 0;
            }
            if (arg == "--scan")
            {
                if (index + 1 >= argc)
                {
                    throw std::runtime_error("--scan attend un index maximum.");
                }
                scan_max_index = std::stoi(argv[++index]);
                continue;
            }
            if (arg == "--output")
            {
                if (index + 1 >= argc)
                {
                    throw std::runtime_error("--output attend un dossier.");
                }
                output_dir = argv[++index];
                continue;
            }
            if (config_path.empty())
            {
                config_path = arg;
                continue;
            }
            throw std::runtime_error("Argument inconnu: " + arg);
        }

        std::vector<CameraProbe> probes;
        if (!config_path.empty())
        {
            const auto config = visiondarts::load_app_config(config_path);
            const auto cameras = enabled_cameras(config);
            if (cameras.empty())
            {
                throw std::runtime_error("Aucune camera active dans la config.");
            }
            for (const auto& camera : cameras)
            {
                probes.push_back(probe_camera(camera, output_dir, true));
            }
        }

        if (scan_max_index >= 0 || config_path.empty())
        {
            const int max_index = scan_max_index >= 0 ? scan_max_index : 10;
            for (int device_index = 0; device_index <= max_index; ++device_index)
            {
                visiondarts::LiveCameraConfig camera;
                camera.camera_id = device_index + 1;
                camera.device_index = device_index;
                probes.push_back(probe_camera(camera, output_dir, false));
            }
        }

        nlohmann::json report;
        report["output_dir"] = output_dir.string();
        report["cameras"] = nlohmann::json::array();

        int opened_count = 0;
        int captured_count = 0;
        int configured_count = 0;
        for (const auto& probe : probes)
        {
            print_probe(probe);
            report["cameras"].push_back(to_json(probe));
            if (probe.configured)
            {
                ++configured_count;
            }
            if (probe.opened)
            {
                ++opened_count;
            }
            if (probe.frame_captured)
            {
                ++captured_count;
            }
        }

        report["configured_count"] = configured_count;
        report["opened_count"] = opened_count;
        report["captured_count"] = captured_count;

        std::filesystem::create_directories(output_dir);
        const auto report_path = output_dir / "camera_diagnostics.json";
        std::ofstream output(report_path);
        output << report.dump(2) << '\n';
        std::cout << "Rapport JSON: " << report_path.string() << '\n';

        if (configured_count > 0 && captured_count < configured_count)
        {
            return 2;
        }
        return captured_count > 0 ? 0 : 2;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Erreur: " << exception.what() << '\n';
        print_usage();
        return 1;
    }
}
