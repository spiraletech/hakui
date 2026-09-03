#include "action/HakuiActionGate.hpp"

#include "core/GameRuntime.hpp"
#include "core/HakuiSnapshot.hpp"

namespace hakui {

HakuiActionCapability HakuiActionGate::requiredCapability(
    HakuiActionVerb verb
) noexcept
{
    switch (verb) {
        case HakuiActionVerb::NpcObservePlayer:
        case HakuiActionVerb::NpcResumeRoutine:
            return HakuiActionCapability::NpcAttention;
    }
    return HakuiActionCapability::None;
}

HakuiActionResult HakuiActionGate::finish(
    const HakuiActionRequest& request,
    HakuiActionStatus status,
    std::string_view detail
) noexcept
{
    ++audit_.evaluated;
    audit_.lastRequestId = request.requestId;
    audit_.lastStatus = status;
    if (status == HakuiActionStatus::Executed) {
        ++audit_.executed;
    } else {
        ++audit_.denied;
    }
    return {request.requestId, status, detail};
}

HakuiActionResult HakuiActionGate::execute(
    GameRuntime& runtime,
    const HakuiActionRequest& request,
    const HakuiActionGrant& grant
) noexcept
{
    if (request.requestId == 0 || request.targetNpcId == 0 ||
        request.snapshotVersion != HakuiSnapshot::schemaVersion) {
        return finish(request, HakuiActionStatus::InvalidRequest,
                      "invalid L11 action envelope");
    }

    const std::uint64_t worldStep = runtime.world().clock().step();
    if (request.observedWorldStep != worldStep) {
        return finish(request, HakuiActionStatus::StaleObservation,
                      "request does not target current world truth");
    }

    const HakuiActionCapability needed = requiredCapability(request.verb);
    const HakuiActionCapabilityMask neededMask = actionCapability(needed);
    if (needed == HakuiActionCapability::None || grant.source != request.source ||
        grant.targetNpcId != request.targetNpcId ||
        worldStep < grant.validFromWorldStep ||
        worldStep > grant.validThroughWorldStep ||
        (grant.capabilities & neededMask) != neededMask) {
        return finish(request, HakuiActionStatus::PermissionDenied,
                      "host grant does not authorize this action");
    }

    if (runtime.npcs().find(request.targetNpcId) == nullptr) {
        return finish(request, HakuiActionStatus::UnknownTarget,
                      "target resident does not exist");
    }

    bool accepted = false;
    switch (request.verb) {
        case HakuiActionVerb::NpcObservePlayer:
            accepted = runtime.npcs().requestObservePlayer(
                request.targetNpcId,
                runtime.player()
            );
            break;
        case HakuiActionVerb::NpcResumeRoutine:
            accepted = runtime.npcs().requestResumeRoutine(request.targetNpcId);
            break;
    }

    return accepted
        ? finish(request, HakuiActionStatus::Executed,
                 "typed action accepted by NPC authority")
        : finish(request, HakuiActionStatus::RejectedByAuthority,
                 "NPC authority rejected action in current state");
}

} // namespace hakui
