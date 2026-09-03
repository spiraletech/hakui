#pragma once

#include "action/HakuiActionGate.hpp"
#include "intent/HakuiIntentProposal.hpp"

namespace hakui {

class GameRuntime;

enum class NpcExecutionStatus : std::uint8_t {
    Executed, NotApproved, UnsupportedVerb, InvalidActor, InvalidTarget,
    DeniedByActionGate
};

struct NpcExecutionApproval {
    std::uint64_t proposalId = 0;
    std::uint64_t actorId = 0;
    std::uint64_t targetId = 0;
    HakuiActionCapabilityMask capabilities = 0;
    std::uint64_t worldStep = 0;
};

struct NpcExecutionResult {
    NpcExecutionStatus status = NpcExecutionStatus::NotApproved;
    HakuiActionStatus actionStatus = HakuiActionStatus::InvalidRequest;
    [[nodiscard]] bool executed() const noexcept {
        return status == NpcExecutionStatus::Executed;
    }
};

class HakuiNpcActionExecutor final {
public:
    [[nodiscard]] NpcExecutionResult execute(
        GameRuntime& runtime,
        const intent::IntentProposal& proposal,
        const NpcExecutionApproval& approval
    ) const noexcept;
};

} // namespace hakui
