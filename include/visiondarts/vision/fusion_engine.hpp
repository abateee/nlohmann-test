#pragma once

#include <vector>

#include "visiondarts/core/types.hpp"

namespace visiondarts
{
class FusionEngine
{
  public:
    ImpactFinal fuse(
        const std::vector<ImpactCamera>& impacts,
        bool allow_single_source,
        double outlier_threshold) const;
};
} // namespace visiondarts
