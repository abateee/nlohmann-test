#include "visiondarts/vision/replay.hpp"

#include <algorithm>
#include <fstream>
#include <stdexcept>

#include <opencv2/imgcodecs.hpp>

#include "visiondarts/core/config.hpp"

namespace visiondarts
{
namespace
{
bool is_scenario_directory(const std::filesystem::path& path)
{
    return std::filesystem::is_directory(path) && std::filesystem::exists(path / "scenario.json");
}
} // namespace

ReplayScenario ReplayFrameSource::load_scenario(const std::filesystem::path& scenario_directory) const
{
    if (!is_scenario_directory(scenario_directory))
    {
        throw std::runtime_error("Le dossier de scenario est invalide: " + scenario_directory.string());
    }

    ReplayScenario scenario;
    scenario.directory = scenario_directory;
    scenario.reference_path = scenario_directory / "reference.png";
    scenario.snapshot_path = scenario_directory / "snapshot.png";
    scenario.calibration_path = std::filesystem::exists(scenario_directory / "calibration.fs.json")
        ? scenario_directory / "calibration.fs.json"
        : scenario_directory / "calibration.json";
    scenario.scenario_path = scenario_directory / "scenario.json";
    scenario.expected_path = scenario_directory / "expected.json";
    scenario.scenario_config = load_scenario_config(scenario.scenario_path);
    scenario.reference_image = cv::imread(scenario.reference_path.string(), cv::IMREAD_COLOR);
    scenario.snapshot_image = cv::imread(scenario.snapshot_path.string(), cv::IMREAD_COLOR);

    if (scenario.reference_image.empty())
    {
        throw std::runtime_error("Impossible de lire reference.png dans " + scenario_directory.string());
    }
    if (scenario.snapshot_image.empty())
    {
        throw std::runtime_error("Impossible de lire snapshot.png dans " + scenario_directory.string());
    }

    if (std::filesystem::exists(scenario.expected_path))
    {
        std::ifstream input(scenario.expected_path);
        scenario.expected_json = nlohmann::json::parse(input);
    }
    else
    {
        scenario.expected_json = nlohmann::json::object();
    }

    return scenario;
}

std::vector<std::filesystem::path> ReplayFrameSource::list_scenarios(const std::filesystem::path& path) const
{
    std::vector<std::filesystem::path> scenarios;

    if (is_scenario_directory(path))
    {
        scenarios.push_back(path);
        return scenarios;
    }

    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("Le chemin de scenario n'existe pas: " + path.string());
    }

    for (const auto& entry : std::filesystem::directory_iterator(path))
    {
        if (is_scenario_directory(entry.path()))
        {
            scenarios.push_back(entry.path());
        }
    }

    std::sort(scenarios.begin(), scenarios.end());
    return scenarios;
}
} // namespace visiondarts

