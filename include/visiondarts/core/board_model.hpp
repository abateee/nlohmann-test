#pragma once

#include "visiondarts/core/types.hpp"

namespace visiondarts
{
class ScoreEngine
{
  public:
    ShotResult score_point(const BoardPoint& point) const;
};
} // namespace visiondarts

