#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <httplib.h>
#include <nlohmann/json.hpp>

#include "visiondarts/app/http_service.hpp"
#include "visiondarts/app/offline_engine.hpp"
#include "visiondarts/app/service_controller.hpp"
#include "visiondarts/core/board_model.hpp"
#include "visiondarts/core/config.hpp"
#include "visiondarts/core/json_utils.hpp"
#include "visiondarts/vision/calibration.hpp"
#include "visiondarts/vision/fusion_engine.hpp"
#include "visiondarts/vision/replay.hpp"

namespace
{
void expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message); // error handling simple 
    }
}

void expect_near(double actual, double expected, double tolerance, const std::string& message)
{
    if (std::abs(actual - expected) > tolerance)
    {
        throw std::runtime_error(message); // encore
    }
}

std::filesystem::path find_repo_root()
{
    auto current = std::filesystem::current_path();
    for (;;)
    {
        if (std::filesystem::exists(current / "CMakeLists.txt") && std::filesystem::exists(current / "fixtures"))
        {
            return current;
        }

        const auto parent = current.parent_path();
        if (parent.empty() || parent == current)
        {
            break;
        }
        current = parent;
    }

    throw std::runtime_error("Impossible de localiser la racine du depot.");
}

class TempDirectory
{
  public:
    explicit TempDirectory(const std::string& prefix)
    {
        static std::uint64_t counter = 0;
        path_ = std::filesystem::temp_directory_path()
            / (prefix + "-" + std::to_string(++counter) + "-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        std::filesystem::create_directories(path_);
    }

    ~TempDirectory()
    {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }

    const std::filesystem::path& path() const
    {
        return path_;
    }

  private:
    std::filesystem::path path_;
};

std::filesystem::path copy_fixture(const std::string& scenario_name, const TempDirectory& temp_directory)
{
    const auto source = find_repo_root() / "fixtures" / scenario_name;
    const auto destination = temp_directory.path() / scenario_name;
    std::filesystem::copy(source, destination, std::filesystem::copy_options::recursive);
    return destination;
}

void wait_for_server(int port)
{
    httplib::Client client("127.0.0.1", port);
    client.set_connection_timeout(0, 200000);
    client.set_read_timeout(0, 200000);

    for (int attempt = 0; attempt < 40; ++attempt)
    {
        if (const auto response = client.Get("/healthcheck"))
        {
            if (response->status == 200)
            {
                return;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    throw std::runtime_error("Le serveur HTTP de test n'a pas demarre.");
}

class TestServiceServer
{
  public:
    explicit TestServiceServer(visiondarts::AppConfig config)
        : controller_(std::move(config))
    {
        visiondarts::configure_service_socket_options(server_);
        visiondarts::register_service_routes(server_, controller_);
        port_ = server_.bind_to_any_port("127.0.0.1");
        expect(port_ > 0, "Impossible de reserver un port pour le serveur de test.");

        worker_ = std::thread([this] {
            server_.listen_after_bind();
        });

        wait_for_server(port_);
    }

    ~TestServiceServer()
    {
        server_.stop();
        if (worker_.joinable())
        {
            worker_.join();
        }
    }

    int port() const
    {
        return port_;
    }

  private:
    visiondarts::ServiceController controller_;
    httplib::Server server_;
    std::thread worker_;
    int port_ = -1;
};

void test_score_engine()
{
    visiondarts::ScoreEngine engine;

    const auto triple_20 = engine.score_point({0.0, 0.60});
    expect(triple_20.segment == "T20", "Le score T20 est incorrect.");
    expect(triple_20.score == 60, "La valeur de T20 est incorrecte.");

    const auto double_20 = engine.score_point({0.0, 0.97});
    expect(double_20.segment == "D20", "Le score D20 est incorrect.");
    expect(double_20.score == 40, "La valeur de D20 est incorrecte.");

    const auto inner_bull = engine.score_point({0.0, 0.0});
    expect(inner_bull.segment == "INNER_BULL", "Le bull interieur est incorrect.");
    expect(inner_bull.score == 50, "La valeur du bull interieur est incorrecte.");

    const auto miss = engine.score_point({1.2, 0.0});
    expect(miss.status == visiondarts::ShotStatus::Invalid, "Le MISS doit etre invalide.");
}

void test_backend_defaults()
{
    const visiondarts::BackendConfig direct_defaults;
    const auto parsed_defaults = nlohmann::json::object().get<visiondarts::BackendConfig>();

    expect(direct_defaults.post_url == "http://127.0.0.1:8080/vision/events", "Le post_url par defaut est incorrect.");
    expect(direct_defaults.service_host == "127.0.0.1", "Le service_host par defaut est incorrect.");
    expect(direct_defaults.service_port == 8090, "Le service_port par defaut est incorrect.");
    expect(direct_defaults.post_timeout_ms == 500, "Le timeout par defaut est incorrect.");
    expect(direct_defaults.post_retry_count == 3, "Le nombre de tentatives par defaut est incorrect.");

    expect(parsed_defaults.post_url == direct_defaults.post_url, "Le post_url par defaut diverge entre construction et parsing JSON.");
    expect(parsed_defaults.service_host == direct_defaults.service_host, "Le service_host par defaut diverge entre construction et parsing JSON.");
    expect(parsed_defaults.service_port == direct_defaults.service_port, "Le service_port par defaut diverge entre construction et parsing JSON.");
    expect(parsed_defaults.post_timeout_ms == direct_defaults.post_timeout_ms, "Le timeout par defaut diverge entre construction et parsing JSON.");
    expect(parsed_defaults.post_retry_count == direct_defaults.post_retry_count, "Le retry_count par defaut diverge entre construction et parsing JSON.");
}

void test_live_config_parsing()
{
    const auto replay_config = nlohmann::json::object().get<visiondarts::AppConfig>();
    expect(replay_config.execution.mode == "replay", "Le mode par defaut doit rester replay.");
    expect(replay_config.live.reference_stability_frames == 5, "Le nombre de frames de reference par defaut est incorrect.");
    expect(replay_config.live.shot_change_threshold == 8.0, "Le seuil de changement live par defaut est incorrect.");
    expect(replay_config.cameras.empty(), "La config par defaut ne doit pas forcer de camera explicite.");

    const nlohmann::json live_json = {
        {"execution", {{"mode", "live"}, {"run_all_on_start", false}}},
        {"live", {
            {"reference_stability_frames", 7},
            {"shot_change_threshold", 12.5},
            {"stabilization_frames", 4},
            {"loop_sleep_ms", 33},
            {"min_ms_between_shots", 900},
        }},
        {"cameras", nlohmann::json::array({
            {
                {"camera_id", 2},
                {"device_index", 1},
                {"width", 1280},
                {"height", 720},
                {"fps", 30},
                {"capture_backend", "msmf"},
                {"calibration_path", "config/calibration-camera-2.json"},
                {"enabled", true},
                {"mask", {{"center_x", 640}, {"center_y", 360}, {"radius_px", 320}}},
            },
            {
                {"camera_id", 3},
                {"device_index", 2},
                {"width", 1280},
                {"height", 720},
                {"fps", 30},
                {"calibration_path", "config/calibration-camera-3.json"},
                {"enabled", true},
            },
            {
                {"camera_id", 4},
                {"device_index", 3},
                {"width", 1280},
                {"height", 720},
                {"fps", 30},
                {"calibration_path", "config/calibration-camera-4.json"},
                {"enabled", false},
            },
        })},
    };

    const auto live_config = live_json.get<visiondarts::AppConfig>();
    expect(live_config.execution.mode == "live", "Le mode live n'est pas parse correctement.");
    expect(!live_config.execution.run_all_on_start, "run_all_on_start live devrait etre false dans ce test.");
    expect(live_config.live.reference_stability_frames == 7, "reference_stability_frames n'est pas parse.");
    expect(live_config.live.shot_change_threshold == 12.5, "shot_change_threshold n'est pas parse.");
    expect(live_config.live.stabilization_frames == 4, "stabilization_frames n'est pas parse.");
    expect(live_config.live.loop_sleep_ms == 33, "loop_sleep_ms n'est pas parse.");
    expect(live_config.live.min_ms_between_shots == 900, "min_ms_between_shots n'est pas parse.");
    expect(live_config.cameras.size() == 3, "La liste cameras live n'est pas parse.");
    expect(live_config.cameras.front().camera_id == 2, "camera_id live incorrect.");
    expect(live_config.cameras.front().device_index == 1, "device_index live incorrect.");
    expect(live_config.cameras.front().width == 1280, "width live incorrect.");
    expect(live_config.cameras.front().height == 720, "height live incorrect.");
    expect(live_config.cameras.front().fps == 30, "fps live incorrect.");
    expect(live_config.cameras.front().capture_backend == "msmf", "capture_backend live incorrect.");
    expect(live_config.cameras.front().calibration_path == std::filesystem::path("config/calibration-camera-2.json"), "calibration_path live incorrect.");
    expect(live_config.cameras.front().mask.has_value(), "mask live devrait etre parse.");
    expect(live_config.cameras.front().mask->radius_px == 320, "mask radius live incorrect.");
    expect(live_config.cameras.at(1).camera_id == 3, "La deuxieme camera live n'est pas parse.");
    expect(!live_config.cameras.at(2).enabled, "La camera desactivee doit rester presente mais inactive.");
}

void test_live_config_validation()
{
    const nlohmann::json duplicate_device_json = {
        {"execution", {{"mode", "live"}}},
        {"cameras", nlohmann::json::array({
            {{"camera_id", 1}, {"device_index", 0}, {"calibration_path", "config/calibration-camera-1.json"}},
            {{"camera_id", 2}, {"device_index", 0}, {"calibration_path", "config/calibration-camera-2.json"}},
        })},
    };

    bool duplicate_device_rejected = false;
    try
    {
        (void)duplicate_device_json.get<visiondarts::AppConfig>();
    }
    catch (const std::exception&)
    {
        duplicate_device_rejected = true;
    }
    expect(duplicate_device_rejected, "La config live doit refuser deux cameras sur le meme device_index.");

    const nlohmann::json too_many_cameras_json = {
        {"execution", {{"mode", "live"}}},
        {"cameras", nlohmann::json::array({
            {{"camera_id", 1}, {"device_index", 0}, {"calibration_path", "config/calibration-camera-1.json"}},
            {{"camera_id", 2}, {"device_index", 1}, {"calibration_path", "config/calibration-camera-2.json"}},
            {{"camera_id", 3}, {"device_index", 2}, {"calibration_path", "config/calibration-camera-3.json"}},
            {{"camera_id", 4}, {"device_index", 3}, {"calibration_path", "config/calibration-camera-4.json"}},
        })},
    };

    bool too_many_cameras_rejected = false;
    try
    {
        (void)too_many_cameras_json.get<visiondarts::AppConfig>();
    }
    catch (const std::exception&)
    {
        too_many_cameras_rejected = true;
    }
    expect(too_many_cameras_rejected, "La config live doit refuser plus de 3 cameras actives.");

    const nlohmann::json invalid_backend_json = {
        {"execution", {{"mode", "live"}}},
        {"cameras", nlohmann::json::array({
            {
                {"camera_id", 1},
                {"device_index", 0},
                {"capture_backend", "unknown"},
                {"calibration_path", "config/calibration-camera-1.json"},
            },
        })},
    };

    bool invalid_backend_rejected = false;
    try
    {
        (void)invalid_backend_json.get<visiondarts::AppConfig>();
    }
    catch (const std::exception&)
    {
        invalid_backend_rejected = true;
    }
    expect(invalid_backend_rejected, "La config live doit refuser un capture_backend inconnu.");
}

void test_live_windows_config_file()
{
    const auto config = visiondarts::load_app_config(find_repo_root() / "config" / "live_windows.json");
    expect(config.execution.mode == "live", "config/live_windows.json doit etre en mode live.");
    expect(config.cameras.size() == 3, "config/live_windows.json doit configurer 3 cameras.");
    expect(config.cameras.at(0).camera_id == 1, "La camera 1 live est mal configuree.");
    expect(config.cameras.at(0).device_index == 1, "Le device_index de la camera 1 est incorrect.");
    expect(config.cameras.at(0).capture_backend == "dshow", "La camera 1 live doit utiliser DirectShow sur Windows.");
    expect(config.cameras.at(1).camera_id == 2, "La camera 2 live est mal configuree.");
    expect(config.cameras.at(1).device_index == 2, "Le device_index de la camera 2 est incorrect.");
    expect(config.cameras.at(1).capture_backend == "dshow", "La camera 2 live doit utiliser DirectShow sur Windows.");
    expect(config.cameras.at(2).camera_id == 3, "La camera 3 live est mal configuree.");
    expect(config.cameras.at(2).device_index == 3, "Le device_index de la camera 3 est incorrect.");
    expect(config.cameras.at(2).capture_backend == "dshow", "La camera 3 live doit utiliser DirectShow sur Windows.");
}

void test_json_subset()
{
    const nlohmann::json actual = {
        {"event", "shot_detected"},
        {"score", 60},
        {"confidence", 0.923},
        {"board_point", {{"x", 0.01}, {"y", -0.02}}},
    };
    const nlohmann::json expected = {
        {"event", "shot_detected"},
        {"score", 60},
        {"confidence", 0.92},
    };

    std::string mismatch_path;
    expect(
        visiondarts::json_contains_expected_subset(actual, expected, 0.01, &mismatch_path),
        "La comparaison de sous-ensemble JSON devrait reussir.");
}

void test_calibration_projection()
{
    const std::vector<cv::Point2d> image_points = {
        {100.0, 100.0},
        {300.0, 100.0},
        {300.0, 300.0},
        {100.0, 300.0},
    };
    const std::vector<cv::Point2d> board_points = {
        {-1.0, 1.0},
        {1.0, 1.0},
        {1.0, -1.0},
        {-1.0, -1.0},
    };

    const auto calibration = visiondarts::compute_calibration(1, image_points, board_points, 0.0);
    const auto board_center = visiondarts::project_image_point(calibration, {200.0, 200.0});
    expect_near(board_center.x, 0.0, 1e-6, "La projection du centre sur X est incorrecte.");
    expect_near(board_center.y, 0.0, 1e-6, "La projection du centre sur Y est incorrecte.");

    const auto board_top = visiondarts::project_image_point(calibration, {200.0, 100.0});
    expect_near(board_top.x, 0.0, 1e-6, "La projection du haut de cible sur X est incorrecte.");
    expect_near(board_top.y, 1.0, 1e-6, "La projection du haut de cible sur Y est incorrecte.");

    const auto rotated_calibration = visiondarts::compute_calibration(1, image_points, board_points, 90.0);
    const auto rotated_top = visiondarts::project_image_point(rotated_calibration, {200.0, 100.0});
    expect_near(rotated_top.x, -1.0, 1e-6, "offset_angle_deg devrait pivoter la projection sur X.");
    expect_near(rotated_top.y, 0.0, 1e-6, "offset_angle_deg devrait pivoter la projection sur Y.");

    const auto image_top = visiondarts::project_board_point_to_image(rotated_calibration, {-1.0, 0.0});
    expect_near(image_top.x, 200.0, 1e-6, "La projection inverse avec rotation est incorrecte sur X.");
    expect_near(image_top.y, 100.0, 1e-6, "La projection inverse avec rotation est incorrecte sur Y.");
}

void test_fusion_engine()
{
    visiondarts::FusionEngine fusion;
    std::vector<visiondarts::ImpactCamera> impacts = {
        {1, {0.10, 0.20}, 0.9, false, true, ""},
        {2, {0.11, 0.19}, 0.8, false, true, ""},
        {3, {0.50, 0.50}, 0.2, false, true, ""},
    };

    const auto result = fusion.fuse(impacts, false, 0.05);
    expect(result.valid, "La fusion aurait du retenir les deux impacts coherents.");
    expect(std::abs(result.point_cible.x - 0.1047) < 0.01, "La fusion sur X est incorrecte.");
}

void test_missing_expected_fixture_behavior()
{
    const visiondarts::ReplayFrameSource frame_source;
    visiondarts::OfflineVisionEngine engine(visiondarts::PipelineConfig{});

    TempDirectory temp_directory("visiondarts-missing-expected");
    const auto scenario_directory = copy_fixture("single_20", temp_directory);
    std::filesystem::remove(scenario_directory / "expected.json");
    const auto scenario = frame_source.load_scenario(scenario_directory);

    visiondarts::OfflineRunOptions replay_options;
    replay_options.compare_expected = true;
    const auto replay_result = engine.run_scenario(scenario, replay_options);
    expect(!replay_result.matches_expected, "L'absence de expected.json doit faire echouer le replay de validation.");
    expect(
        replay_result.actual_event.at("event").get<std::string>() == "vision_error",
        "Le replay doit signaler une vision_error si expected.json manque.");
    expect(
        replay_result.actual_event.at("code").get<std::string>() == "missing_expected_fixture",
        "Le replay doit utiliser le code missing_expected_fixture.");

    visiondarts::OfflineRunOptions service_options;
    service_options.compare_expected = false;
    service_options.allow_single_source = true;
    const auto service_result = engine.run_scenario(scenario, service_options);
    expect(
        service_result.actual_event.at("event").get<std::string>() == "shot_detected",
        "Le mode service doit accepter un scenario sans expected.json.");
}

void test_calibration_missing_and_corrupt()
{
    const visiondarts::ReplayFrameSource frame_source;
    visiondarts::OfflineVisionEngine engine(visiondarts::PipelineConfig{});

    {
        TempDirectory temp_directory("visiondarts-missing-calibration");
        const auto scenario_directory = copy_fixture("single_20", temp_directory);
        std::filesystem::remove(scenario_directory / "calibration.json");
        const auto scenario = frame_source.load_scenario(scenario_directory);

        visiondarts::OfflineRunOptions options;
        options.compare_expected = false;
        const auto result = engine.run_scenario(scenario, options);
        expect(
            result.actual_event.at("event").get<std::string>() == "calibration_required",
            "Une calibration manquante doit produire calibration_required.");
    }

    {
        TempDirectory temp_directory("visiondarts-corrupt-calibration");
        const auto scenario_directory = copy_fixture("single_20", temp_directory);
        std::ofstream output(scenario_directory / "calibration.json", std::ios::trunc);
        output << "{\"camera_id\":1,\"points_image\": [";
        output.close();

        const auto scenario = frame_source.load_scenario(scenario_directory);
        visiondarts::OfflineRunOptions options;
        options.compare_expected = false;
        const auto result = engine.run_scenario(scenario, options);
        expect(
            result.actual_event.at("event").get<std::string>() == "vision_error",
            "Une calibration corrompue doit produire une vision_error.");
        expect(
            result.actual_event.at("code").get<std::string>() == "calibration_load_failure",
            "Une calibration corrompue doit utiliser le code calibration_load_failure.");
    }
}

void test_service_routes()
{
    visiondarts::AppConfig config;
    TestServiceServer server(config);
    httplib::Client client("127.0.0.1", server.port());
    client.set_connection_timeout(0, 200000);
    client.set_read_timeout(0, 200000);

    const auto bad_json_response = client.Post("/commands/calibrate", "{bad json", "application/json");
    expect(static_cast<bool>(bad_json_response), "La route /commands/calibrate doit repondre.");
    expect(bad_json_response->status == 400, "Le JSON invalide doit retourner HTTP 400.");
    const auto bad_json_body = nlohmann::json::parse(bad_json_response->body);
    expect(!bad_json_body.at("accepted").get<bool>(), "La reponse invalid_json doit etre refusee.");
    expect(bad_json_body.at("error").get<std::string>() == "invalid_json", "Le code invalid_json est incorrect.");

    const auto reset_response = client.Post("/commands/reset-reference", "", "application/json");
    expect(static_cast<bool>(reset_response), "La route /commands/reset-reference doit repondre.");
    expect(reset_response->status == 501, "reset-reference doit retourner HTTP 501 en mode offline.");
    const auto reset_body = nlohmann::json::parse(reset_response->body);
    expect(!reset_body.at("accepted").get<bool>(), "reset-reference ne doit pas etre annonce comme accepte.");
    expect(
        reset_body.at("error").get<std::string>() == "unsupported_in_offline_mode",
        "Le code d'erreur reset-reference est incorrect.");
}

void test_service_socket_exclusivity()
{
    httplib::Server first_server;
    visiondarts::configure_service_socket_options(first_server);
    const int port = first_server.bind_to_any_port("127.0.0.1");
    expect(port > 0, "Le premier bind de test doit reussir.");

    httplib::Server second_server;
    visiondarts::configure_service_socket_options(second_server);
    expect(
        !second_server.bind_to_port("127.0.0.1", port),
        "Le second bind sur le meme port devrait echouer.");

    first_server.stop();
    second_server.stop();
}
} // namespace

int main()
{
    try
    {
        test_score_engine();
        test_backend_defaults();
        test_live_config_parsing();
        test_live_config_validation();
        test_live_windows_config_file();
        test_json_subset();
        test_calibration_projection();
        test_fusion_engine();
        test_missing_expected_fixture_behavior();
        test_calibration_missing_and_corrupt();
        test_service_routes();
        test_service_socket_exclusivity();
        std::cout << "Tous les tests unitaires ont reussi.\n";
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Echec de test: " << exception.what() << '\n';
        return 1;
    }
}
