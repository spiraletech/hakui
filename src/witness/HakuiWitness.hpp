#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>
#include <string_view>
#include <vector>

namespace hakui::witness {

enum class WitnessKind : std::uint8_t {
    Input,
    Routing,
    Observation,
    Decision,
    Mutation,
    Result,
    Limitation
};

enum class WitnessKnowledge : std::uint8_t {
    Observed,
    Inferred,
    Unknown
};

struct WitnessEntry {
    std::uint64_t sequence = 0;
    std::uint64_t worldStep = 0;
    float elapsedSeconds = 0.0f;
    WitnessKind kind = WitnessKind::Observation;
    WitnessKnowledge knowledge = WitnessKnowledge::Observed;
    std::string category;
    std::string detail;
};

struct WitnessSnapshot {
    static constexpr std::uint32_t schemaVersion = 1;

    std::uint32_t version = schemaVersion;
    std::size_t capacity = 0;
    std::uint64_t recorded = 0;
    std::uint64_t dropped = 0;
    std::vector<WitnessEntry> entries;
};

// L12's bounded session ledger. Callers supply deterministic world time; the
// witness never reads a wall clock, executes gameplay, or retains references
// into authoritative state. Snapshots are owned copies safe for inspection.
class HakuiWitness final {
public:
    explicit HakuiWitness(std::size_t capacity = 256) noexcept;

    void record(
        std::uint64_t worldStep,
        float elapsedSeconds,
        WitnessKind kind,
        WitnessKnowledge knowledge,
        std::string_view category,
        std::string_view detail
    );

    void observed(
        std::uint64_t worldStep,
        float elapsedSeconds,
        WitnessKind kind,
        std::string_view category,
        std::string_view detail
    );

    void inferred(
        std::uint64_t worldStep,
        float elapsedSeconds,
        WitnessKind kind,
        std::string_view category,
        std::string_view detail
    );

    void unknown(
        std::uint64_t worldStep,
        float elapsedSeconds,
        WitnessKind kind,
        std::string_view category,
        std::string_view detail
    );

    [[nodiscard]] WitnessSnapshot snapshot() const;
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] std::size_t size() const noexcept { return entries_.size(); }
    [[nodiscard]] std::uint64_t recorded() const noexcept { return recorded_; }
    [[nodiscard]] std::uint64_t dropped() const noexcept { return dropped_; }

    void clear() noexcept;

private:
    std::size_t capacity_ = 256;
    std::uint64_t nextSequence_ = 1;
    std::uint64_t recorded_ = 0;
    std::uint64_t dropped_ = 0;
    std::deque<WitnessEntry> entries_;
};

[[nodiscard]] std::string_view witnessKindLabel(WitnessKind kind) noexcept;
[[nodiscard]] std::string_view witnessKnowledgeLabel(
    WitnessKnowledge knowledge
) noexcept;

} // namespace hakui::witness
