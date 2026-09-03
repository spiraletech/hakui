#include "navigation/HakuiNavigation.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <queue>
#include <vector>

namespace hakui::navigation {

NavigationPath HakuiNavigation::plan(
    const MovementEnvironment& env,
    NavigationPoint start,
    NavigationPoint goal
) const
{
    NavigationPath result;
    if (!std::isfinite(start.x) || !std::isfinite(start.z) ||
        !std::isfinite(goal.x) || !std::isfinite(goal.z) ||
        config_.cellSize <= 0.0f || config_.maxExpandedCells == 0)
        return result;

    const int width = static_cast<int>(std::floor(
        (env.floorMaximumX - env.floorMinimumX) / config_.cellSize)) + 1;
    const int depth = static_cast<int>(std::floor(
        (env.floorMaximumZ - env.floorMinimumZ) / config_.cellSize)) + 1;
    if (width <= 0 || depth <= 0 ||
        static_cast<std::size_t>(width) * static_cast<std::size_t>(depth) >
            config_.maxExpandedCells)
        return result;

    const auto index = [width](int x, int z) { return z * width + x; };
    const auto point = [&](int x, int z) {
        return NavigationPoint{env.floorMinimumX + x * config_.cellSize,
                               env.floorMinimumZ + z * config_.cellSize};
    };
    const auto coordinate = [&](float value, float minimum, int limit) {
        return std::clamp(static_cast<int>(std::lround(
            (value - minimum) / config_.cellSize)), 0, limit - 1);
    };
    const auto open = [&](int x, int z) {
        const NavigationPoint p = point(x, z);
        if (!env.hasFloorAt(p.x, p.z)) return false;
        for (const HorizontalCollider& collider : env.colliders) {
            if (p.x >= collider.minimumX - config_.agentRadius &&
                p.x <= collider.maximumX + config_.agentRadius &&
                p.z >= collider.minimumZ - config_.agentRadius &&
                p.z <= collider.maximumZ + config_.agentRadius)
                return false;
        }
        return true;
    };

    const int sx = coordinate(start.x, env.floorMinimumX, width);
    const int sz = coordinate(start.z, env.floorMinimumZ, depth);
    const int gx = coordinate(goal.x, env.floorMinimumX, width);
    const int gz = coordinate(goal.z, env.floorMinimumZ, depth);
    if (!open(sx, sz) || !open(gx, gz)) return result;

    const int cellCount = width * depth;
    std::vector<int> parent(static_cast<std::size_t>(cellCount), -1);
    std::queue<int> frontier;
    const int startIndex = index(sx, sz);
    const int goalIndex = index(gx, gz);
    parent[static_cast<std::size_t>(startIndex)] = startIndex;
    frontier.push(startIndex);
    constexpr std::array<int, 4> dx{1, 0, -1, 0};
    constexpr std::array<int, 4> dz{0, 1, 0, -1};
    std::size_t expanded = 0;
    while (!frontier.empty() && parent[static_cast<std::size_t>(goalIndex)] < 0 &&
           expanded++ < config_.maxExpandedCells) {
        const int current = frontier.front(); frontier.pop();
        const int cx = current % width;
        const int cz = current / width;
        for (std::size_t direction = 0; direction < dx.size(); ++direction) {
            const int nx = cx + dx[direction], nz = cz + dz[direction];
            if (nx < 0 || nx >= width || nz < 0 || nz >= depth || !open(nx, nz))
                continue;
            const int next = index(nx, nz);
            if (parent[static_cast<std::size_t>(next)] >= 0) continue;
            parent[static_cast<std::size_t>(next)] = current;
            frontier.push(next);
        }
    }
    if (parent[static_cast<std::size_t>(goalIndex)] < 0) return result;

    std::vector<int> reverse;
    for (int cursor = goalIndex; cursor != startIndex;
         cursor = parent[static_cast<std::size_t>(cursor)])
        reverse.push_back(cursor);
    std::reverse(reverse.begin(), reverse.end());
    int previous = startIndex;
    int previousDx = 0;
    int previousDz = 0;
    for (std::size_t pathIndex = 0; pathIndex < reverse.size(); ++pathIndex) {
        const int cell = reverse[pathIndex];
        const int directionX = cell % width - previous % width;
        const int directionZ = cell / width - previous / width;
        if (pathIndex > 0 &&
            (directionX != previousDx || directionZ != previousDz)) {
            if (result.count == NavigationPath::maxWaypoints)
                return NavigationPath{};
            result.waypoints[result.count++] =
                point(previous % width, previous / width);
        }
        previousDx = directionX;
        previousDz = directionZ;
        previous = cell;
    }
    if (result.count == NavigationPath::maxWaypoints)
        return NavigationPath{};
    result.waypoints[result.count++] = point(gx, gz);
    result.waypoints[result.count - 1] = goal;
    result.complete = true;
    return result;
}
} // namespace hakui::navigation
