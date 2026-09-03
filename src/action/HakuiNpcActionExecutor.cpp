#include "action/HakuiNpcActionExecutor.hpp"

#include "core/GameRuntime.hpp"
#include "core/HakuiSnapshot.hpp"
#include "npc/NpcManager.hpp"

namespace hakui {

NpcExecutionResult HakuiNpcActionExecutor::execute(
    GameRuntime& runtime,
    const intent::IntentProposal& proposal,
    const NpcExecutionApproval& approval
) const noexcept
{
    if (approval.proposalId != proposal.id || approval.actorId != proposal.actorId ||
        approval.targetId != proposal.targetId ||
        approval.worldStep != runtime.world().clock().step())
        return {NpcExecutionStatus::NotApproved, HakuiActionStatus::PermissionDenied};
    if (proposal.actorId != NpcManager::saelisId)
        return {NpcExecutionStatus::InvalidActor, HakuiActionStatus::UnknownTarget};
    if (proposal.targetId != 1)
        return {NpcExecutionStatus::InvalidTarget, HakuiActionStatus::UnknownTarget};
    if (proposal.verb != intent::IntentVerb::LookAt)
        return {NpcExecutionStatus::UnsupportedVerb, HakuiActionStatus::InvalidRequest};

    const std::uint64_t step = runtime.world().clock().step();
    const HakuiActionRequest request{
        proposal.id, approval.source,
        HakuiActionVerb::NpcObservePlayer,
        static_cast<std::uint32_t>(proposal.actorId),
        HakuiSnapshot::schemaVersion, step
    };
    const HakuiActionGrant grant{
        approval.source,
        static_cast<std::uint32_t>(proposal.actorId),
        approval.capabilities, step, step
    };
    const HakuiActionResult action = runtime.actionGate().execute(runtime, request, grant);
    return {action.executed() ? NpcExecutionStatus::Executed
                              : NpcExecutionStatus::DeniedByActionGate,
            action.status};
}
} // namespace hakui
