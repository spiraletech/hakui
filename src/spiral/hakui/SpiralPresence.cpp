#include "spiral/hakui/SpiralPresence.hpp"

#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

namespace hakui {
namespace {

const HakuiAffordanceSnapshot* findPresenceNode(
    const HakuiWorldSnapshot& world
) noexcept
{
    for (const HakuiAffordanceSnapshot& affordance : world.affordances) {
        if (affordance.id == SpiralPresence::nodeAffordanceId) {
            return &affordance;
        }
    }
    return nullptr;
}

std::string fixed1(float value)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(1) << value;
    return stream.str();
}

} // namespace

SpiralPresenceView SpiralPresence::view(float nearbyRadius) const
{
    SpiralPresenceView view;
    view.readOnly = true;
    view.cortexBound = false;
    view.interactionRadius = nodeInteractionRadius;
    view.headline = "SPIRAL // HAKUI PRESENCE";
    view.linkLine = "HAKUI LINK // READ ONLY";
    view.cortexLine = "CORTEX // UNBOUND // L9";

    const HakuiSnapshot snapshot = adapter_.snapshot();
    view.snapshotVersion = snapshot.version;

    const HakuiAffordanceSnapshot* node = findPresenceNode(snapshot.world);
    if (node) {
        view.nodeX = node->primaryAnchor.x;
        view.nodeY = node->primaryAnchor.y;
        view.nodeZ = node->primaryAnchor.z;

        const float dx = snapshot.player.x - view.nodeX;
        const float dy = snapshot.player.y - view.nodeY;
        const float dz = snapshot.player.z - view.nodeZ;
        view.playerDistance = std::sqrt(dx * dx + dy * dy + dz * dz);
        view.playerInRange = view.playerDistance <= nodeInteractionRadius;
    } else {
        view.playerDistance = std::numeric_limits<float>::infinity();
    }

    const float safeRadius =
        std::isfinite(nearbyRadius) && nearbyRadius >= 0.0f
            ? nearbyRadius
            : defaultNearbyRadius;
    const HakuiNearbyInspection nearby = adapter_.inspectNearby(safeRadius);
    view.nearbyObjectCount = nearby.objects.size();

    view.linked = node != nullptr &&
        adapter_.supports(HakuiAdapter::inspectWorldCommand) &&
        adapter_.supports(HakuiAdapter::inspectPlayerCommand) &&
        adapter_.supports(HakuiAdapter::inspectNearbyCommand);

    view.worldLine = "WORLD " + snapshot.world.worldId +
        " // STEP " + std::to_string(snapshot.world.simulationStep);
    view.playerLine = "PLAYER " + snapshot.player.displayName +
        " // HP " + fixed1(snapshot.player.health) +
        " // STA " + fixed1(snapshot.player.stamina);
    view.nearbyLine = "NEARBY " + std::to_string(view.nearbyObjectCount) +
        " // SNAPSHOT V" + std::to_string(snapshot.version);

    return view;
}

} // namespace hakui
