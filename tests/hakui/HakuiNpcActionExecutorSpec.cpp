#include "action/HakuiNpcActionExecutor.hpp"
#include "core/GameRuntime.hpp"
#include "npc/NpcManager.hpp"
#include <cassert>

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
    assert(executor.execute(runtime, walk, approval).status ==
           NpcExecutionStatus::UnsupportedVerb);
    return 0;
}
