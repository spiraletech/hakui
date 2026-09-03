#include "action/HakuiActionGate.hpp"
#include "core/GameRuntime.hpp"
#include "core/HakuiSnapshot.hpp"
#include "npc/NpcManager.hpp"

#include <cassert>

namespace {

hakui::HakuiActionRequest observeRequest(
    const hakui::GameRuntime& runtime,
    std::uint64_t requestId
)
{
    return {
        requestId,
        hakui::HakuiActionSource::Cortex,
        hakui::HakuiActionVerb::NpcObservePlayer,
        hakui::NpcManager::saelisId,
        hakui::HakuiSnapshot::schemaVersion,
        runtime.world().clock().step()
    };
}

hakui::HakuiActionGrant attentionGrant(const hakui::GameRuntime& runtime)
{
    return {
        hakui::HakuiActionSource::Cortex,
        hakui::NpcManager::saelisId,
        hakui::actionCapability(hakui::HakuiActionCapability::NpcAttention),
        runtime.world().clock().step(),
        runtime.world().clock().step()
    };
}

} // namespace

int main()
{
    using namespace hakui;

    GameRuntime runtime;
    HakuiActionGate& gate = runtime.actionGate();
    NpcState* saelis = runtime.npcs().find(NpcManager::saelisId);
    assert(saelis != nullptr);

    HakuiActionRequest request = observeRequest(runtime, 1);
    HakuiActionGrant noGrant{};
    const float originalYaw = saelis->yaw;
    HakuiActionResult denied = gate.execute(runtime, request, noGrant);
    assert(denied.status == HakuiActionStatus::PermissionDenied);
    assert(saelis->activity == NpcActivity::Walking);
    assert(saelis->yaw == originalYaw);
    assert(runtime.witness().recorded() == 1);
    assert(runtime.witness().snapshot().entries.back().category == "action.gate");

    HakuiActionGrant grant = attentionGrant(runtime);
    HakuiActionResult accepted = gate.execute(runtime, request, grant);
    assert(accepted.executed());
    assert(saelis->activity == NpcActivity::ObservingPlayer);

    runtime.advanceWorld(0.05f);
    HakuiActionResult stale = gate.execute(runtime, request, grant);
    assert(stale.status == HakuiActionStatus::StaleObservation);

    request = observeRequest(runtime, 2);
    grant = attentionGrant(runtime);
    grant.targetNpcId = 9999;
    assert(gate.execute(runtime, request, grant).status ==
           HakuiActionStatus::PermissionDenied);

    request.targetNpcId = 9999;
    grant.targetNpcId = 9999;
    assert(gate.execute(runtime, request, grant).status ==
           HakuiActionStatus::UnknownTarget);

    const HakuiActionAudit& audit = gate.audit();
    assert(audit.evaluated == 5);
    assert(audit.executed == 1);
    assert(audit.denied == 4);
    assert(audit.lastRequestId == 2);
    const witness::WitnessSnapshot witness = runtime.witness().snapshot();
    assert(witness.recorded == audit.evaluated);
    assert(witness.entries.back().kind == witness::WitnessKind::Decision);
    assert(witness.entries.back().detail.find("target resident does not exist") !=
           std::string::npos);
    return 0;
}
