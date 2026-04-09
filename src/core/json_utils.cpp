#include "visiondarts/core/json_utils.hpp"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace visiondarts
{
namespace
{
bool compare_json_recursive(
    const nlohmann::json& actual,
    const nlohmann::json& expected,
    const std::string& current_path,
    double numeric_tolerance,
    std::string* mismatch_path)
{
    if (expected.is_number() && actual.is_number())
    {
        const double actual_value = actual.get<double>();
        const double expected_value = expected.get<double>();
        if (std::abs(actual_value - expected_value) > numeric_tolerance)
        {
            if (mismatch_path != nullptr)
            {
                *mismatch_path = current_path;
            }
            return false;
        }
        return true;
    }

    if (expected.type() != actual.type())
    {
        if (mismatch_path != nullptr)
        {
            *mismatch_path = current_path;
        }
        return false;
    }

    if (expected.is_object())
    {
        for (auto it = expected.begin(); it != expected.end(); ++it)
        {
            if (!actual.contains(it.key()))
            {
                if (mismatch_path != nullptr)
                {
                    *mismatch_path = current_path + "/" + it.key();
                }
                return false;
            }

            if (!compare_json_recursive(
                    actual.at(it.key()),
                    it.value(),
                    current_path + "/" + it.key(),
                    numeric_tolerance,
                    mismatch_path))
            {
                return false;
            }
        }

        return true;
    }

    if (expected.is_array())
    {
        if (actual.size() != expected.size())
        {
            if (mismatch_path != nullptr)
            {
                *mismatch_path = current_path;
            }
            return false;
        }

        for (std::size_t index = 0; index < expected.size(); ++index)
        {
            if (!compare_json_recursive(
                    actual.at(index),
                    expected.at(index),
                    current_path + "/" + std::to_string(index),
                    numeric_tolerance,
                    mismatch_path))
            {
                return false;
            }
        }

        return true;
    }

    if (actual != expected)
    {
        if (mismatch_path != nullptr)
        {
            *mismatch_path = current_path;
        }
        return false;
    }

    return true;
}
} // namespace

void to_json(nlohmann::json& j, const BoardPoint& point)
{
    j = nlohmann::json{
        {"x", point.x},
        {"y", point.y},
        {"space", "board_normalized"},
    };
}

void from_json(const nlohmann::json& j, BoardPoint& point)
{
    point.x = j.at("x").get<double>();
    point.y = j.at("y").get<double>();
}

void to_json(nlohmann::json& j, const ImpactCamera& impact)
{
    j = nlohmann::json{
        {"camera_id", impact.camera_id},
        {"x", impact.point_cible.x},
        {"y", impact.point_cible.y},
        {"quality", impact.quality},
        {"used_in_fusion", impact.used_in_fusion},
        {"valid", impact.valid},
        {"reason", impact.reason},
    };
}

void from_json(const nlohmann::json& j, ImpactCamera& impact)
{
    impact.camera_id = j.at("camera_id").get<int>();
    impact.point_cible.x = j.at("x").get<double>();
    impact.point_cible.y = j.at("y").get<double>();
    impact.quality = j.value("quality", 0.0);
    impact.used_in_fusion = j.value("used_in_fusion", false);
    impact.valid = j.value("valid", false);
    impact.reason = j.value("reason", std::string{});
}

void to_json(nlohmann::json& j, const ShotResult& shot)
{
    j = nlohmann::json{
        {"event", shot.event},
        {"schema_version", shot.schema_version},
        {"shot_id", shot.shot_id},
        {"timestamp_ms", shot.timestamp_ms},
        {"status", to_string(shot.status)},
        {"segment", shot.segment},
        {"score", shot.score},
        {"ring", to_string(shot.ring)},
        {"board_point", shot.board_point},
        {"confidence", shot.confidence},
        {"processing_ms", shot.processing_ms},
        {"cameras_expected", shot.cameras_expected},
        {"cameras_used", shot.cameras_used},
        {"camera_impacts", shot.camera_impacts},
    };

    if (shot.sector.has_value())
    {
        j["sector"] = *shot.sector;
    }
    else
    {
        j["sector"] = nullptr;
    }

    if (shot.multiplier.has_value())
    {
        j["multiplier"] = *shot.multiplier;
    }
    else
    {
        j["multiplier"] = nullptr;
    }

    if (shot.reason.has_value())
    {
        j["reason"] = *shot.reason;
    }
}

void to_json(nlohmann::json& j, const VisionError& error)
{
    j = nlohmann::json{
        {"event", error.event},
        {"schema_version", error.schema_version},
        {"timestamp_ms", error.timestamp_ms},
        {"code", error.code},
        {"message", error.message},
    };
}

void to_json(nlohmann::json& j, const CalibrationRequired& missing)
{
    j = nlohmann::json{
        {"event", missing.event},
        {"schema_version", missing.schema_version},
        {"timestamp_ms", missing.timestamp_ms},
        {"missing_camera_ids", missing.missing_camera_ids},
    };
}

bool json_contains_expected_subset(
    const nlohmann::json& actual,
    const nlohmann::json& expected,
    double numeric_tolerance,
    std::string* mismatch_path)
{
    return compare_json_recursive(actual, expected, "$", numeric_tolerance, mismatch_path);
}

std::string make_shot_id(std::uint64_t sequence)
{
    std::ostringstream stream;
    stream << "shot-" << std::setfill('0') << std::setw(6) << sequence;
    return stream.str();
}

std::int64_t unix_timestamp_ms()
{
    const auto now = std::chrono::system_clock::now();
    const auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return millis.count();
}
} // namespace visiondarts

