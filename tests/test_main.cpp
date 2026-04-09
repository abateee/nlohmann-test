#include <cmath>
#include <iostream>
#include <stdexcept>

#include <nlohmann/json.hpp>

#include "visiondarts/core/board_model.hpp"
#include "visiondarts/core/json_utils.hpp"
#include "visiondarts/vision/calibration.hpp"
#include "visiondarts/vision/fusion_engine.hpp"

namespace
{
void expect(bool condition, const std::string& message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

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
    expect(std::abs(board_center.x) < 1e-6, "La projection du centre sur X est incorrecte.");
    expect(std::abs(board_center.y) < 1e-6, "La projection du centre sur Y est incorrecte.");
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
} // namespace

int main()
{
    try
    {
        test_score_engine();
        test_json_subset();
        test_calibration_projection();
        test_fusion_engine();
        std::cout << "Tous les tests unitaires ont reussi.\n";
        return 0;
    }
    catch (const std::exception& exception)
    {
        std::cerr << "Echec de test: " << exception.what() << '\n';
        return 1;
    }
}
