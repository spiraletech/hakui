#include "spiral/hakui/SpiralPresence.hpp"

#include <cassert>
#include <cmath>
#include <string>
#include <type_traits>

namespace {

bool near(float left, float right, float epsilon = 0.0001f) noexcept
{
    return std::fabs(left - right) <= epsilon;
}

const hakui::HakuiAffordanceSnapshot* findNode(
    const hakui::HakuiWorldSnapshot& world
) noexcept
{
    for (const auto& affordance : world.affordances) {
        if (affordance.id == hakui::SpiralPresence::nodeAffordanceId) {
            return &affordance;
        }
    }
    return nullptr;
}

} // namespace

int main()
{
    static_assert(std::is_constructible_v<
        hakui::SpiralPresence,
        const hakui::HakuiAdapter&
    >);
    static_assert(!std::is_constructible_v<
        hakui::SpiralPresence,
        hakui::HakuiAdapter&&
    >);

    hakui::GameRuntime runtime;
    runtime.resetSession(500.0f);
    runtime.advanceWorld(3.0f);
    runtime.player().displayName = "L8 TEST";
    runtime.player().health = 91.0f;
    runtime.player().stamina = 74.0f;

    const hakui::HakuiAdapter adapter(runtime);
    const hakui::SpiralPresence presence(adapter);

    const hakui::HakuiSnapshot before = adapter.snapshot();
    const hakui::HakuiAffordanceSnapshot* node = findNode(before.world);

    // L8 invariant 1: the visible presence is backed by an authored semantic
    // terminal node that is visible to the same L6/L7 truth path as Spiral.
    assert(node != nullptr);
    assert(hakui::hasAffordance(
        node->affordances,
        hakui::WorldAffordance::Terminal
    ));

    const hakui::SpiralPresenceView initial = presence.view();
    assert(initial.linked);
    assert(initial.readOnly);
    assert(!initial.cortexBound);
    assert(initial.snapshotVersion == hakui::HakuiSnapshot::schemaVersion);
    assert(near(initial.nodeX, node->primaryAnchor.x));
    assert(near(initial.nodeY, node->primaryAnchor.y));
    assert(near(initial.nodeZ, node->primaryAnchor.z));
    assert(initial.headline == "SPIRAL // HAKUI PRESENCE");
    assert(initial.linkLine == "HAKUI LINK // READ ONLY");
    assert(initial.cortexLine.find("UNBOUND") != std::string::npos);
    assert(initial.worldLine.find(before.world.worldId) != std::string::npos);
    assert(initial.playerLine.find("L8 TEST") != std::string::npos);

    // L8 invariant 2: proximity is observation only. Moving the authoritative
    // player near the node changes the next presence view, not vice versa.
    runtime.player().x = node->primaryAnchor.x;
    runtime.player().y = node->primaryAnchor.y;
    runtime.player().z = node->primaryAnchor.z + 1.0f;

    const hakui::SpiralPresenceView nearby = presence.view(4.0f);
    assert(nearby.playerInRange);
    assert(nearby.playerDistance <= hakui::SpiralPresence::nodeInteractionRadius);
    assert(nearby.nearbyObjectCount > 0);

    // L8 invariant 3: repeated presence refreshes do not mutate HAKUI truth.
    const hakui::HakuiSnapshot observationStart = adapter.snapshot();
    for (int index = 0; index < 8; ++index) {
        (void)presence.view(6.0f);
    }
    const hakui::HakuiSnapshot observationEnd = adapter.snapshot();

    assert(observationStart.world.simulationStep == observationEnd.world.simulationStep);
    assert(near(observationStart.world.elapsedSeconds, observationEnd.world.elapsedSeconds));
    assert(near(observationStart.player.x, observationEnd.player.x));
    assert(near(observationStart.player.y, observationEnd.player.y));
    assert(near(observationStart.player.z, observationEnd.player.z));
    assert(near(observationStart.player.health, observationEnd.player.health));
    assert(near(observationStart.player.money, observationEnd.player.money));
    assert(observationStart.world.occupiedSeatCount == observationEnd.world.occupiedSeatCount);
    assert(observationStart.interactions.liveTargetIds == observationEnd.interactions.liveTargetIds);

    // L8 invariant 4: presence never claims a cortex before L9.
    runtime.player().health = 17.0f;
    runtime.advanceWorld(0.5f);
    const hakui::SpiralPresenceView later = presence.view();
    assert(later.linked);
    assert(!later.cortexBound);
    assert(later.playerLine.find("17.0") != std::string::npos);
    assert(later.worldLine.find("STEP 2") != std::string::npos);

    return 0;
}
