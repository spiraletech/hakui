#include "core/HakuiSnapshot.hpp"

#include <cassert>
#include <cmath>
#include <limits>

namespace {

bool near(float left, float right, float epsilon = 0.001f) noexcept
{
    return std::fabs(left - right) <= epsilon;
}

const hakui::HakuiNpcSnapshot* findNpc(
    const hakui::HakuiSnapshot& snapshot,
    std::uint32_t id
) noexcept
{
    for (const auto& npc : snapshot.npcs) {
        if (npc.id == id) {
            return &npc;
        }
    }
    return nullptr;
}

} // namespace

int main()
{
    hakui::GameRuntime runtime;
    runtime.resetSession(250.0f);

    assert(runtime.npcs().size() == 1);
    hakui::NpcState* saelis = runtime.npcs().find(hakui::NpcManager::saelisId);
    assert(saelis != nullptr);
    assert(saelis->displayName == "SAELIS");
    assert(saelis->bodyProfile == hakui::avatar::BodyProfileId::Female);
    assert(saelis->activity == hakui::NpcActivity::Walking);
    assert(saelis->routine == hakui::NpcRoutinePhase::WalkToCouch);
    assert(near(saelis->movementBlend, 0.0f));
    assert(!saelis->seatOccupancy);

    const float spawnX = saelis->x;
    const float spawnZ = saelis->z;
    const std::uint64_t initialTicks = saelis->simulationTicks;
    const std::uint64_t initialWorldStep = runtime.world().clock().step();

    // L10 invariant 1: resident simulation accepts only deltas accepted by the
    // canonical world clock. Invalid time cannot mutate NPC state independently.
    runtime.advanceWorld(std::numeric_limits<float>::quiet_NaN());
    runtime.advanceWorld(-1.0f);
    assert(runtime.world().clock().step() == initialWorldStep);
    assert(saelis->simulationTicks == initialTicks);
    assert(near(saelis->x, spawnX));
    assert(near(saelis->z, spawnZ));

    // Keep the player outside notice range while proving the authored routine.
    runtime.player().x = 100.0f;
    runtime.player().z = 100.0f;

    // L10 invariant 2: Saelis independently walks to the lounge and reserves a
    // real BlackRoom seat. NPC occupancy is the same room truth player seating
    // uses; there is no private NPC-only furniture state.
    bool reachedCouch = false;
    for (int step = 0; step < 160 && !reachedCouch; ++step) {
        runtime.advanceWorld(0.1f);
        reachedCouch = saelis->activity == hakui::NpcActivity::Seated;
    }
    assert(reachedCouch);
    assert(saelis->routine == hakui::NpcRoutinePhase::CouchRest);
    runtime.advanceWorld(1.0f);
    assert(saelis->movementBlend < 0.01f);
    assert(saelis->seatOccupancy);
    assert(saelis->activeAffordanceId == 1002);
    assert(saelis->activeSeatAnchorId != 0);
    assert(runtime.blackRoom().seatOccupied(saelis->activeSeatAnchorId));

    const std::uint32_t npcSeat = saelis->activeSeatAnchorId;
    const hakui::ResolvedSeatAnchor resolved =
        runtime.blackRoom().resolvedSeatAnchor(npcSeat);
    assert(resolved.id == npcSeat);
    assert(near(saelis->x, resolved.worldPosition.x));
    assert(near(saelis->y, resolved.worldPosition.y));
    assert(near(saelis->z, resolved.worldPosition.z));

    const hakui::HakuiSnapshot seatedSnapshot =
        hakui::captureHakuiSnapshot(runtime);
    assert(seatedSnapshot.version == hakui::HakuiSnapshot::schemaVersion);
    assert(seatedSnapshot.version == 2);
    const hakui::HakuiNpcSnapshot* seatedSaelis =
        findNpc(seatedSnapshot, hakui::NpcManager::saelisId);
    assert(seatedSaelis != nullptr);
    assert(seatedSaelis->bodyProfile == hakui::avatar::BodyProfileId::Female);
    assert(seatedSaelis->seatOccupancy);
    assert(seatedSaelis->activeSeatAnchorId == npcSeat);
    assert(seatedSnapshot.world.occupiedSeatCount == 1);

    // L10 invariant 3: session reset clears resident reservations and restores
    // the authored resident identity/routine without changing NPC authority.
    runtime.resetSession(250.0f);
    saelis = runtime.npcs().find(hakui::NpcManager::saelisId);
    assert(saelis != nullptr);
    assert(near(saelis->x, spawnX));
    assert(near(saelis->z, spawnZ));
    assert(saelis->simulationTicks == 0);
    assert(saelis->routineCycles == 0);
    assert(!saelis->seatOccupancy);
    assert(!runtime.blackRoom().seatOccupied(npcSeat));

    runtime.player().x = 100.0f;
    runtime.player().z = 100.0f;
    reachedCouch = false;
    for (int step = 0; step < 160 && !reachedCouch; ++step) {
        runtime.advanceWorld(0.1f);
        reachedCouch = saelis->activity == hakui::NpcActivity::Seated;
    }
    assert(reachedCouch);

    // Rest completion must release the shared seat before the resident resumes
    // her route toward the Spiral node.
    const std::uint32_t secondSeat = saelis->activeSeatAnchorId;
    runtime.advanceWorld(6.1f);
    assert(!saelis->seatOccupancy);
    assert(saelis->activeSeatAnchorId == 0);
    assert(!runtime.blackRoom().seatOccupied(secondSeat));
    assert(saelis->routine == hakui::NpcRoutinePhase::WalkSouthBypass);
    assert(saelis->activity == hakui::NpcActivity::Walking);

    // L10 invariant 4: residents can react to authoritative player proximity
    // without a language model. The reaction is bounded and the deterministic
    // routine resumes afterward.
    runtime.player().x = saelis->x + 0.50f;
    runtime.player().y = saelis->y;
    runtime.player().z = saelis->z;
    runtime.advanceWorld(0.1f);
    assert(saelis->activity == hakui::NpcActivity::ObservingPlayer);
    assert(saelis->mood == hakui::NpcMood::Curious);

    runtime.player().x = 100.0f;
    runtime.player().z = 100.0f;
    runtime.advanceWorld(1.3f);
    assert(saelis->activity == hakui::NpcActivity::Walking);
    assert(saelis->routine == hakui::NpcRoutinePhase::WalkSouthBypass);

    // L10 invariant 5: the routine continues through a navigation bypass to the
    // authored Spiral presence and enters a model-independent observe state.
    bool observedSpiral = false;
    for (int step = 0; step < 360 && !observedSpiral; ++step) {
        runtime.advanceWorld(0.1f);
        observedSpiral =
            saelis->activity == hakui::NpcActivity::ObservingSpiral &&
            saelis->routine == hakui::NpcRoutinePhase::ObserveSpiral;
    }
    assert(observedSpiral);
    assert(saelis->mood == hakui::NpcMood::Curious);
    assert(!saelis->seatOccupancy);

    // Snapshot resident truth is an owned frozen copy just like world/player
    // truth. Later simulation cannot mutate an older cortex/observer packet.
    const hakui::HakuiSnapshot frozen = hakui::captureHakuiSnapshot(runtime);
    const hakui::HakuiNpcSnapshot* frozenSaelis =
        findNpc(frozen, hakui::NpcManager::saelisId);
    assert(frozenSaelis != nullptr);
    const float frozenX = frozenSaelis->x;
    const std::uint64_t frozenTicks = frozenSaelis->simulationTicks;

    runtime.advanceWorld(4.2f);
    const hakui::HakuiSnapshot later = hakui::captureHakuiSnapshot(runtime);
    const hakui::HakuiNpcSnapshot* laterSaelis =
        findNpc(later, hakui::NpcManager::saelisId);
    assert(laterSaelis != nullptr);
    assert(near(frozenSaelis->x, frozenX));
    assert(frozenSaelis->simulationTicks == frozenTicks);
    assert(laterSaelis->simulationTicks > frozenTicks);

    return 0;
}
