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
    runtime.player().displayName = "L9 TEST";
    runtime.player().health = 91.0f;
    runtime.player().stamina = 74.0f;

    const hakui::HakuiAdapter adapter(runtime);
    const hakui::SpiralPresence presence(adapter);

    const hakui::HakuiSnapshot before = adapter.snapshot();
    const hakui::HakuiAffordanceSnapshot* node = findNode(before.world);

    assert(node != nullptr);
    assert(hakui::hasAffordance(
        node->affordances,
        hakui::WorldAffordance::Terminal
    ));

    const hakui::SpiralPresenceView initial = presence.view();
    assert(initial.linked);
    assert(initial.readOnly);
    assert(!initial.cortexBound);
    assert(!initial.cortexBusy);
    assert(!initial.cortexLocalModelLoaded);
    assert(initial.snapshotVersion == hakui::HakuiSnapshot::schemaVersion);
    assert(near(initial.nodeX, node->primaryAnchor.x));
    assert(near(initial.nodeY, node->primaryAnchor.y));
    assert(near(initial.nodeZ, node->primaryAnchor.z));
    assert(initial.headline == "SPIRAL // HAKUI PRESENCE");
    assert(initial.linkLine == "HAKUI LINK // READ ONLY");
    assert(initial.cortexLine.find("OFFLINE") != std::string::npos);
    assert(initial.worldLine.find(before.world.worldId) != std::string::npos);
    assert(initial.playerLine.find("L9 TEST") != std::string::npos);

    // Proximity is still observation only.
    runtime.player().x = node->primaryAnchor.x;
    runtime.player().y = node->primaryAnchor.y;
    runtime.player().z = node->primaryAnchor.z + 1.0f;

    const hakui::SpiralPresenceView nearby = presence.view(4.0f);
    assert(nearby.playerInRange);
    assert(nearby.playerDistance <= hakui::SpiralPresence::nodeInteractionRadius);
    assert(nearby.nearbyObjectCount > 0);

    // L9 can display a bound real cortex state without granting it authority.
    hakui::SpiralCortexStatus bound;
    bound.bound = true;
    bound.localModelLoaded = true;
    bound.runtimeName = "Spiral Ether AI";
    bound.model = "test.gguf";
    const hakui::SpiralPresenceView boundView = presence.view(6.0f, bound);
    assert(boundView.readOnly);
    assert(boundView.cortexBound);
    assert(boundView.cortexLocalModelLoaded);
    assert(boundView.cortexLine.find("BOUND") != std::string::npos);
    assert(boundView.cortexLine.find("LOCAL MODEL") != std::string::npos);

    bound.busy = true;
    const hakui::SpiralPresenceView thinking = presence.view(6.0f, bound);
    assert(thinking.cortexBound);
    assert(thinking.cortexBusy);
    assert(thinking.cortexLine.find("THINKING") != std::string::npos);

    // Repeated presence refreshes, including cortex status changes, do not
    // mutate authoritative HAKUI truth.
    const hakui::HakuiSnapshot observationStart = adapter.snapshot();
    for (int index = 0; index < 8; ++index) {
        (void)presence.view(6.0f, bound);
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

    runtime.player().health = 17.0f;
    runtime.advanceWorld(0.5f);
    const hakui::SpiralPresenceView later = presence.view(6.0f, bound);
    assert(later.linked);
    assert(later.cortexBound);
    assert(later.playerLine.find("17.0") != std::string::npos);
    assert(later.worldLine.find("STEP 2") != std::string::npos);

    return 0;
}
