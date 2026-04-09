#include "visiondarts/core/board_model.hpp"

#include <algorithm>
#include <array>
#include <cmath>

#include "visiondarts/core/json_utils.hpp"

namespace visiondarts
{
namespace
{
constexpr double kPi = 3.14159265358979323846;
constexpr double kDoubleOuterRadius = 1.0;
constexpr double kDoubleInnerRadius = 162.0 / 170.0;
constexpr double kTripleOuterRadius = 107.0 / 170.0;
constexpr double kTripleInnerRadius = 99.0 / 170.0;
constexpr double kOuterBullRadius = 15.9 / 170.0;
constexpr double kInnerBullRadius = 6.35 / 170.0;
constexpr std::array<int, 20> kSectorOrder = {
    20, 1, 18, 4, 13, 6, 10, 15, 2, 17,
    3, 19, 7, 16, 8, 11, 14, 9, 12, 5};

double clamp01(double value)
{
    return std::max(0.0, std::min(1.0, value));
}
} // namespace

ShotResult ScoreEngine::score_point(const BoardPoint& point) const
{
    ShotResult result;
    result.shot_id = make_shot_id(0);
    result.timestamp_ms = unix_timestamp_ms();
    result.board_point = point;

    const double radius = std::sqrt((point.x * point.x) + (point.y * point.y));
    if (radius > kDoubleOuterRadius)
    {
        result.status = ShotStatus::Invalid;
        result.segment = "MISS";
        result.ring = Ring::Miss;
        result.reason = "impact_outside_board";
        result.confidence = 0.0;
        return result;
    }

    if (radius <= kInnerBullRadius)
    {
        result.status = ShotStatus::Valid;
        result.segment = "INNER_BULL";
        result.score = 50;
        result.ring = Ring::InnerBull;
        result.sector.reset();
        result.multiplier.reset();
        result.confidence = clamp01(1.0 - radius);
        return result;
    }

    if (radius <= kOuterBullRadius)
    {
        result.status = ShotStatus::Valid;
        result.segment = "OUTER_BULL";
        result.score = 25;
        result.ring = Ring::OuterBull;
        result.sector.reset();
        result.multiplier.reset();
        result.confidence = clamp01(1.0 - radius);
        return result;
    }

    const double sector_width = (2.0 * kPi) / 20.0;
    const double angle = std::atan2(point.x, point.y);
    const double normalized_angle = angle < 0.0 ? angle + (2.0 * kPi) : angle;
    const int sector_index = static_cast<int>(std::floor((normalized_angle + (sector_width / 2.0)) / sector_width)) % 20;
    const int sector = kSectorOrder.at(static_cast<std::size_t>(sector_index));

    result.sector = sector;
    result.status = ShotStatus::Valid;

    if (radius >= kDoubleInnerRadius)
    {
        result.ring = Ring::Double;
        result.multiplier = 2;
        result.score = sector * 2;
        result.segment = "D" + std::to_string(sector);
    }
    else if (radius >= kTripleInnerRadius && radius <= kTripleOuterRadius)
    {
        result.ring = Ring::Triple;
        result.multiplier = 3;
        result.score = sector * 3;
        result.segment = "T" + std::to_string(sector);
    }
    else
    {
        result.ring = Ring::Single;
        result.multiplier = 1;
        result.score = sector;
        result.segment = "S" + std::to_string(sector);
    }

    const double radial_margin = std::min(radius, std::abs(kDoubleOuterRadius - radius));
    result.confidence = clamp01(0.65 + (0.35 * radial_margin));
    return result;
}
} // namespace visiondarts
