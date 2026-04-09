#pragma once

#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>

#include "visiondarts/core/types.hpp"

namespace visiondarts
{
void to_json(nlohmann::json& j, const BoardPoint& point);
void from_json(const nlohmann::json& j, BoardPoint& point);

void to_json(nlohmann::json& j, const ImpactCamera& impact);
void from_json(const nlohmann::json& j, ImpactCamera& impact);

void to_json(nlohmann::json& j, const ShotResult& shot);
void to_json(nlohmann::json& j, const VisionError& error);
void to_json(nlohmann::json& j, const CalibrationRequired& missing);

bool json_contains_expected_subset(
    const nlohmann::json& actual,
    const nlohmann::json& expected,
    double numeric_tolerance,
    std::string* mismatch_path = nullptr);

std::string make_shot_id(std::uint64_t sequence);
std::int64_t unix_timestamp_ms();
} // namespace visiondarts

