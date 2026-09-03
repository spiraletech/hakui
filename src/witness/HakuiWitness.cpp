#include "witness/HakuiWitness.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace hakui::witness {

HakuiWitness::HakuiWitness(std::size_t capacity) noexcept
    : capacity_(std::max<std::size_t>(capacity, 1))
{
}

void HakuiWitness::record(
    std::uint64_t worldStep,
    float elapsedSeconds,
    WitnessKind kind,
    WitnessKnowledge knowledge,
    std::string_view category,
    std::string_view detail
)
{
    WitnessEntry entry;
    entry.sequence = nextSequence_++;
    entry.worldStep = worldStep;
    entry.elapsedSeconds = std::isfinite(elapsedSeconds)
        ? std::max(elapsedSeconds, 0.0f)
        : 0.0f;
    entry.kind = kind;
    entry.knowledge = knowledge;
    entry.category = category;
    entry.detail = detail;

    ++recorded_;
    if (entries_.size() == capacity_) {
        entries_.pop_front();
        ++dropped_;
    }
    entries_.push_back(std::move(entry));
}

void HakuiWitness::observed(
    std::uint64_t worldStep,
    float elapsedSeconds,
    WitnessKind kind,
    std::string_view category,
    std::string_view detail
)
{
    record(
        worldStep,
        elapsedSeconds,
        kind,
        WitnessKnowledge::Observed,
        category,
        detail
    );
}

void HakuiWitness::inferred(
    std::uint64_t worldStep,
    float elapsedSeconds,
    WitnessKind kind,
    std::string_view category,
    std::string_view detail
)
{
    record(
        worldStep,
        elapsedSeconds,
        kind,
        WitnessKnowledge::Inferred,
        category,
        detail
    );
}

void HakuiWitness::unknown(
    std::uint64_t worldStep,
    float elapsedSeconds,
    WitnessKind kind,
    std::string_view category,
    std::string_view detail
)
{
    record(
        worldStep,
        elapsedSeconds,
        kind,
        WitnessKnowledge::Unknown,
        category,
        detail
    );
}

WitnessSnapshot HakuiWitness::snapshot() const
{
    WitnessSnapshot copy;
    copy.capacity = capacity_;
    copy.recorded = recorded_;
    copy.dropped = dropped_;
    copy.entries.assign(entries_.begin(), entries_.end());
    return copy;
}

void HakuiWitness::clear() noexcept
{
    entries_.clear();
    nextSequence_ = 1;
    recorded_ = 0;
    dropped_ = 0;
}

std::string_view witnessKindLabel(WitnessKind kind) noexcept
{
    switch (kind) {
        case WitnessKind::Input: return "input";
        case WitnessKind::Routing: return "routing";
        case WitnessKind::Observation: return "observation";
        case WitnessKind::Decision: return "decision";
        case WitnessKind::Mutation: return "mutation";
        case WitnessKind::Result: return "result";
        case WitnessKind::Limitation: return "limitation";
    }
    return "observation";
}

std::string_view witnessKnowledgeLabel(WitnessKnowledge knowledge) noexcept
{
    switch (knowledge) {
        case WitnessKnowledge::Observed: return "observed";
        case WitnessKnowledge::Inferred: return "inferred";
        case WitnessKnowledge::Unknown: return "unknown";
    }
    return "unknown";
}

} // namespace hakui::witness
