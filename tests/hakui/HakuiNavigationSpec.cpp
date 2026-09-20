#include "navigation/HakuiNavigation.hpp"
#include "world/BlackRoom.hpp"

#include <cassert>
#include <cmath>

int main()
{
    using namespace hakui;
    using namespace hakui::navigation;

    const BlackRoom room;
    const MovementEnvironment environment = room.movementEnvironment();
    const HakuiNavigation navigation;
    const NavigationPoint goal{0.0f, 5.25f};
    const NavigationPath path = navigation.plan(
        environment, {-4.60f, 3.35f}, goal);

    assert(path.complete);
    assert(path.count > 0 && path.count <= NavigationPath::maxWaypoints);
    assert(std::abs(path.waypoints[path.count - 1].x - goal.x) < 0.001f);
    assert(std::abs(path.waypoints[path.count - 1].z - goal.z) < 0.001f);
    for (std::size_t index = 0; index < path.count; ++index) {
        const NavigationPoint point = path.waypoints[index];
        assert(environment.hasFloorAt(point.x, point.z));
        for (const HorizontalCollider& collider : environment.colliders) {
            const bool insideExpanded =
                point.x >= collider.minimumX - 0.34f &&
                point.x <= collider.maximumX + 0.34f &&
                point.z >= collider.minimumZ - 0.34f &&
                point.z <= collider.maximumZ + 0.34f;
            assert(!insideExpanded);
        }
    }

    const NavigationPath blocked = navigation.plan(
        environment, {-4.60f, 3.35f}, {0.0f, 0.0f});
    assert(!blocked.complete);
    assert(blocked.count == 0);
    return 0;
}
