#include "action/HakuiNpcActionExecutor.hpp"
#include "core/GameRuntime.hpp"
#include "npc/NpcManager.hpp"
#include <cassert>
#include <cmath>

int main()
{
    using namespace hakui;
    GameRuntime runtime;
    HakuiNpcActionExecutor executor;
    const intent::IntentProposal look{7, intent::IntentVerb::LookAt,
        NpcManager::saelisId, 1, "player", "test"};
    NpcExecutionApproval approval{7, NpcManager::saelisId, 1,
        actionCapability(HakuiActionCapability::NpcAttention), 0,
        HakuiActionSource::LocalSystem};

    NpcExecutionApproval missing = approval;
    missing.capabilities = 0;
    assert(!executor.execute(runtime, look, missing).executed());
    assert(runtime.npcs().find(NpcManager::saelisId)->activity == NpcActivity::Walking);

    const NpcExecutionResult accepted = executor.execute(runtime, look, approval);
    assert(accepted.executed());
    assert(runtime.npcs().find(NpcManager::saelisId)->activity ==
           NpcActivity::ObservingPlayer);

    intent::IntentProposal walk = look;
    walk.id = 8;
    walk.verb = intent::IntentVerb::WalkTo;
    approval.proposalId = 8;
    approval.capabilities = actionCapability(HakuiActionCapability::NpcNavigation);
    runtime.player().x = runtime.blackRoom().movementEnvironment().spawnX;
    runtime.player().z = runtime.blackRoom().movementEnvironment().spawnZ;
    assert(executor.execute(runtime, walk, approval).executed());
    const NpcState* saelis = runtime.npcs().find(NpcManager::saelisId);
    assert(saelis && saelis->navigationCommandActive);
    for (int tick = 0; tick < 240 && saelis->navigationCommandActive; ++tick) {
        runtime.advanceWorld(0.1f);
        saelis = runtime.npcs().find(NpcManager::saelisId);
    }
    assert(saelis && !saelis->navigationCommandActive);
    assert(saelis->activity == NpcActivity::ObservingPlayer);
    const float dx = saelis->x - runtime.player().x;
    const float dz = saelis->z - runtime.player().z;
    const float distance = std::sqrt(dx * dx + dz * dz);
    assert(distance >= 0.70f && distance <= 1.10f);
    return 0;
}
