#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace visiondarts
{
enum class Ring
{
    Miss,
    Single,
    Double,
    Triple,
    OuterBull,
    InnerBull
};

enum class ShotStatus
{
    Valid,
    Invalid,
    Uncertain
};

enum class ServiceState
{
    Idle,
    Running,
    Calibrating,
    Error
};

struct BoardPoint
{
    double x = 0.0;
    double y = 0.0;
};

struct ImpactCamera
{
    int camera_id = 0;
    BoardPoint point_cible{};
    double quality = 0.0;
    bool used_in_fusion = false;
    bool valid = false;
    std::string reason;
};

struct ImpactFinal
{
    BoardPoint point_cible{};
    double confidence = 0.0;
    bool valid = false;
    std::string reason;
    std::vector<ImpactCamera> camera_impacts;
};

struct ShotResult
{
    std::string event = "shot_detected";
    int schema_version = 1;
    std::string shot_id;
    std::int64_t timestamp_ms = 0;
    ShotStatus status = ShotStatus::Invalid;
    std::string segment = "MISS";
    int score = 0;
    std::optional<int> sector;
    Ring ring = Ring::Miss;
    std::optional<int> multiplier;
    BoardPoint board_point{};
    double confidence = 0.0;
    int processing_ms = 0;
    int cameras_expected = 1;
    int cameras_used = 0;
    std::vector<ImpactCamera> camera_impacts;
    std::optional<std::string> reason;
};

struct VisionError
{
    std::string event = "vision_error";
    int schema_version = 1;
    std::int64_t timestamp_ms = 0;
    std::string code;
    std::string message;
};

struct CalibrationRequired
{
    std::string event = "calibration_required";
    int schema_version = 1;
    std::int64_t timestamp_ms = 0;
    std::vector<int> missing_camera_ids;
};

std::string to_string(Ring ring);
std::string to_string(ShotStatus status);
std::string to_string(ServiceState state);

Ring ring_from_string(const std::string& value);
ShotStatus shot_status_from_string(const std::string& value);
ServiceState service_state_from_string(const std::string& value);
} // namespace visiondarts

