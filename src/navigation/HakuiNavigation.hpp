#pragma once

#include "player/PlayerMovementController.hpp"

#include <array>
#include <cstddef>

namespace hakui::navigation {

struct NavigationPoint { float x = 0.0f; float z = 0.0f; };

struct NavigationPath {
    static constexpr std::size_t maxWaypoints = 64;
    std::array<NavigationPoint, maxWaypoints> waypoints{};
    std::size_t count = 0;
    bool complete = false;
};

class HakuiNavigation final {
public:
    struct Config {
        float cellSize = 0.50f;
        float agentRadius = 0.34f;
        std::size_t maxExpandedCells = 4096;
    };

    HakuiNavigation() = default;
    explicit HakuiNavigation(Config config) noexcept : config_(config) {}

    [[nodiscard]] NavigationPath plan(
        const MovementEnvironment& environment,
        NavigationPoint start,
        NavigationPoint goal
    ) const;

private:
    Config config_{};
};

} // namespace hakui::navigation
