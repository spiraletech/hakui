#include "witness/HakuiWitness.hpp"

#include <cassert>
#include <cmath>
#include <limits>

int main()
{
    using namespace hakui::witness;

    HakuiWitness witness{3};
    witness.observed(4, 1.25f, WitnessKind::Input, "input.interact", "pressed");
    witness.inferred(4, 1.25f, WitnessKind::Decision, "route", "nearby couch");
    witness.unknown(
        4,
        std::numeric_limits<float>::quiet_NaN(),
        WitnessKind::Limitation,
        "cortex.model",
        "model identity unavailable"
    );

    const WitnessSnapshot frozen = witness.snapshot();
    assert(frozen.version == WitnessSnapshot::schemaVersion);
    assert(frozen.capacity == 3);
    assert(frozen.recorded == 3);
    assert(frozen.dropped == 0);
    assert(frozen.entries.size() == 3);
    assert(frozen.entries[0].sequence == 1);
    assert(frozen.entries[0].knowledge == WitnessKnowledge::Observed);
    assert(frozen.entries[1].knowledge == WitnessKnowledge::Inferred);
    assert(frozen.entries[2].knowledge == WitnessKnowledge::Unknown);
    assert(std::isfinite(frozen.entries[2].elapsedSeconds));

    witness.observed(5, 2.0f, WitnessKind::Mutation, "seat", "reserved");
    const WitnessSnapshot wrapped = witness.snapshot();
    assert(wrapped.recorded == 4);
    assert(wrapped.dropped == 1);
    assert(wrapped.entries.size() == 3);
    assert(wrapped.entries.front().sequence == 2);
    assert(wrapped.entries.back().sequence == 4);

    // Previously captured reports are owned copies.
    assert(frozen.entries.front().sequence == 1);
    assert(frozen.entries.back().detail == "model identity unavailable");

    assert(witnessKindLabel(WitnessKind::Routing) == "routing");
    assert(witnessKnowledgeLabel(WitnessKnowledge::Unknown) == "unknown");

    witness.clear();
    assert(witness.size() == 0);
    assert(witness.recorded() == 0);
    assert(witness.dropped() == 0);

    HakuiWitness minimum{0};
    assert(minimum.capacity() == 1);
    return 0;
}
