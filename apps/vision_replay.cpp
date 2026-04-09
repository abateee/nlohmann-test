#include <filesystem>
#include <iostream>

#include "visiondarts/app/offline_engine.hpp"
#include "visiondarts/vision/replay.hpp"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: vision_replay <scenario-ou-dossier> [--debug-out <dossier>]\n";
        return 1;
    }

    std::filesystem::path scenario_path = argv[1];
    std::filesystem::path debug_output = "build/debug_output";
    bool save_debug = false;
    for (int index = 2; index < argc; ++index)
    {
        const std::string argument = argv[index];
        if (argument == "--debug-out" && (index + 1) < argc)
        {
            debug_output = argv[++index];
            save_debug = true;
        }
    }

    try
    {
        visiondarts::ReplayFrameSource frame_source;
        visiondarts::OfflineVisionEngine engine(visiondarts::PipelineConfig{});
        visiondarts::OfflineRunOptions options;
        options.compare_expected = true;
        options.save_debug = save_debug;
        options.debug_output_root = debug_output;
        options.allow_single_source = true;

        const auto scenarios = frame_source.list_scenarios(scenario_path);
        bool success = true;
        std::uint64_t sequence = 1;
        for (const auto& scenario_dir : scenarios)
        {
            auto scenario = frame_source.load_scenario(scenario_dir);
            options.shot_sequence = sequence++;
            const auto result = engine.run_scenario(scenario, options);
            std::cout << result.actual_event.dump(2) << "\n";
            if (!result.matches_expected)
            {
                success = false;
                std::cerr << "Ecart detecte dans le scenario '" << result.scenario_name
                          << "' au chemin JSON " << result.mismatch_path << ".\n";
            }
        }

        return success ? 0 : 1;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Erreur: " << exception.what() << '\n';
        return 1;
    }
}

