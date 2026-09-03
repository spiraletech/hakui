#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace hakui::intent {

enum class IntentVerb : std::uint8_t { LookAt, WalkTo, Sit, Talk, Inspect };

struct IntentProposal {
    std::uint64_t id = 0;
    IntentVerb verb = IntentVerb::Inspect;
    std::uint64_t actorId = 0;
    std::uint64_t targetId = 0;
    std::string argument;
    std::string evidence;
};

struct ProposalBatch {
    std::vector<IntentProposal> proposals;
    std::size_t rejected = 0;
    bool truncated = false;
};

// Parses explicit, line-oriented proposal envelopes from cortex text. Output is
// data only: this type has no GameRuntime or action-gate execution capability.
class IntentProposalParser final {
public:
    static constexpr std::size_t maxProposals = 8;
    static constexpr std::size_t maxArgumentBytes = 96;

    [[nodiscard]] static ProposalBatch parse(
        std::string_view cortexText,
        std::uint64_t firstProposalId = 1
    );
};

[[nodiscard]] std::string_view intentVerbLabel(IntentVerb verb) noexcept;

} // namespace hakui::intent
