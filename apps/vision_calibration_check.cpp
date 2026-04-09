#include <iostream>

#include <nlohmann/json.hpp>

#include "visiondarts/vision/calibration.hpp"

int main(int argc, char** argv)
{
    if (argc < 4 || ((argc - 2) % 2) != 0)
    {
        std::cerr << "Usage: vision_calibration_check <calibration> <x1> <y1> [<x2> <y2> ...]\n";
        return 1;
    }

    try
    {
        const auto calibration = visiondarts::CalibrationStore::load(argv[1]);
        nlohmann::json output = nlohmann::json::array();
        for (int index = 2; index < argc; index += 2)
        {
            const cv::Point2d image_point(std::stod(argv[index]), std::stod(argv[index + 1]));
            const auto board_point = visiondarts::project_image_point(calibration, image_point);
            output.push_back({
                {"image", {{"x", image_point.x}, {"y", image_point.y}}},
                {"board", {{"x", board_point.x}, {"y", board_point.y}}},
            });
        }

        std::cout << output.dump(2) << '\n';
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Erreur: " << exception.what() << '\n';
        return 1;
    }
}

