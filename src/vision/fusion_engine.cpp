#include "visiondarts/vision/fusion_engine.hpp"

#include <algorithm>
#include <cmath>

namespace visiondarts
{
namespace
{
double clamp01(double value)
{
    return std::max(0.0, std::min(1.0, value));
}

double distance_between(const BoardPoint& a, const BoardPoint& b)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return std::sqrt((dx * dx) + (dy * dy));
}

BoardPoint weighted_mean(const std::vector<ImpactCamera>& impacts)
{
    double sum_weights = 0.0;
    double sum_x = 0.0;
    double sum_y = 0.0;
    for (const auto& impact : impacts)
    {
        const double weight = std::max(impact.quality, 0.001);
        sum_weights += weight;
        sum_x += impact.point_cible.x * weight;
        sum_y += impact.point_cible.y * weight;
    }

    return BoardPoint{sum_x / sum_weights, sum_y / sum_weights};
}
} // namespace

ImpactFinal FusionEngine::fuse(
    const std::vector<ImpactCamera>& impacts,
    bool allow_single_source,
    double outlier_threshold) const
{
    ImpactFinal result;
    result.camera_impacts = impacts;

    std::vector<ImpactCamera> valid_impacts;
    for (const auto& impact : impacts)
    {
        if (impact.valid)
        {
            valid_impacts.push_back(impact);
        }
    }

    if (valid_impacts.empty())
    {
        result.valid = false;
        result.reason = "no_valid_impacts";
        return result;
    }

    if (valid_impacts.size() == 1)
    {
        if (!allow_single_source)
        {
            result.valid = false;
            result.reason = "not_enough_consistent_cameras";
            return result;
        }

        result.valid = true;
        result.point_cible = valid_impacts.front().point_cible;
        result.confidence = clamp01(valid_impacts.front().quality);
        result.camera_impacts.front().used_in_fusion = true;
        return result;
    }

    std::vector<ImpactCamera> cluster;
    for (std::size_t anchor = 0; anchor < valid_impacts.size(); ++anchor)
    {
        std::vector<ImpactCamera> candidate_cluster;
        candidate_cluster.push_back(valid_impacts.at(anchor));

        for (std::size_t other = 0; other < valid_impacts.size(); ++other)
        {
            if (anchor == other)
            {
                continue;
            }

            if (distance_between(valid_impacts.at(anchor).point_cible, valid_impacts.at(other).point_cible) <= outlier_threshold)
            {
                candidate_cluster.push_back(valid_impacts.at(other));
            }
        }

        if (candidate_cluster.size() > cluster.size())
        {
            cluster = candidate_cluster;
        }
    }

    std::sort(cluster.begin(), cluster.end(), [](const ImpactCamera& left, const ImpactCamera& right) {
        return left.camera_id < right.camera_id;
    });

    if (cluster.size() < 2)
    {
        result.valid = false;
        result.reason = "not_enough_consistent_cameras";
        return result;
    }

    result.valid = true;
    result.point_cible = weighted_mean(cluster);

    double mean_quality = 0.0;
    double mean_distance = 0.0;
    for (const auto& impact : cluster)
    {
        mean_quality += impact.quality;
        mean_distance += distance_between(impact.point_cible, result.point_cible);
    }
    mean_quality /= static_cast<double>(cluster.size());
    mean_distance /= static_cast<double>(cluster.size());
    result.confidence = clamp01(mean_quality * (1.0 - std::min(1.0, mean_distance / outlier_threshold)));

    for (auto& impact : result.camera_impacts)
    {
        impact.used_in_fusion = false;
        for (const auto& selected : cluster)
        {
            if (impact.camera_id == selected.camera_id)
            {
                impact.used_in_fusion = true;
            }
        }
    }

    return result;
}
} // namespace visiondarts

