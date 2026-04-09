#include "visiondarts/core/types.hpp"

#include <stdexcept>

namespace visiondarts
{
namespace
{
template <typename EnumType>
[[noreturn]] void throw_bad_value(const std::string& kind, const std::string& value)
{
    throw std::runtime_error("Valeur " + kind + " invalide: " + value);
}
} // namespace

std::string to_string(Ring ring)
{
    switch (ring)
    {
    case Ring::Miss:
        return "MISS";
    case Ring::Single:
        return "SINGLE";
    case Ring::Double:
        return "DOUBLE";
    case Ring::Triple:
        return "TRIPLE";
    case Ring::OuterBull:
        return "OUTER_BULL";
    case Ring::InnerBull:
        return "INNER_BULL";
    }

    return "MISS";
}

std::string to_string(ShotStatus status)
{
    switch (status)
    {
    case ShotStatus::Valid:
        return "valid";
    case ShotStatus::Invalid:
        return "invalid";
    case ShotStatus::Uncertain:
        return "uncertain";
    }

    return "invalid";
}

std::string to_string(ServiceState state)
{
    switch (state)
    {
    case ServiceState::Idle:
        return "idle";
    case ServiceState::Running:
        return "running";
    case ServiceState::Calibrating:
        return "calibrating";
    case ServiceState::Error:
        return "error";
    }

    return "error";
}

Ring ring_from_string(const std::string& value)
{
    if (value == "MISS")
    {
        return Ring::Miss;
    }
    if (value == "SINGLE")
    {
        return Ring::Single;
    }
    if (value == "DOUBLE")
    {
        return Ring::Double;
    }
    if (value == "TRIPLE")
    {
        return Ring::Triple;
    }
    if (value == "OUTER_BULL")
    {
        return Ring::OuterBull;
    }
    if (value == "INNER_BULL")
    {
        return Ring::InnerBull;
    }

    throw_bad_value<Ring>("anneau", value);
}

ShotStatus shot_status_from_string(const std::string& value)
{
    if (value == "valid")
    {
        return ShotStatus::Valid;
    }
    if (value == "invalid")
    {
        return ShotStatus::Invalid;
    }
    if (value == "uncertain")
    {
        return ShotStatus::Uncertain;
    }

    throw_bad_value<ShotStatus>("status", value);
}

ServiceState service_state_from_string(const std::string& value)
{
    if (value == "idle")
    {
        return ServiceState::Idle;
    }
    if (value == "running")
    {
        return ServiceState::Running;
    }
    if (value == "calibrating")
    {
        return ServiceState::Calibrating;
    }
    if (value == "error")
    {
        return ServiceState::Error;
    }

    throw_bad_value<ServiceState>("etat de service", value);
}
} // namespace visiondarts

