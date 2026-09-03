#pragma once

#include <cstdint>
#include <string>

namespace hakui {

enum class NpcActivity : std::uint8_t {
    Idle,
    Walking,
    Seated,
    ObservingPlayer,
    ObservingSpiral
};

enum class NpcMood : std::uint8_t {
    Calm,
    Curious,
    Resting
};

enum class NpcRoutinePhase : std::uint8_t {
    WalkToCouch,
    CouchRest,
    WalkSouthBypass,
    WalkToSpiral,
    ObserveSpiral,
    WalkNorthBypass,
    WalkHome,
    IdleHome
};

struct NpcNeeds {
    float hunger = 100.0f;
    float energy = 92.0f;
    float social = 72.0f;
    float comfort = 68.0f;
    float fun = 74.0f;
};

// L10 deterministic resident state. This is objective HAKUI simulation truth,
// not language-model context or character prose. A cortex may observe a copy of
// this state later, but it does not own these values or the routine clock.
struct NpcState {
    std::uint32_t id = 0;
    std::string displayName;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float yaw = 0.0f;

    float velocityX = 0.0f;
    float velocityZ = 0.0f;

    float homeX = 0.0f;
    float homeY = 0.0f;
    float homeZ = 0.0f;

    float targetX = 0.0f;
    float targetY = 0.0f;
    float targetZ = 0.0f;

    NpcActivity activity = NpcActivity::Idle;
    NpcMood mood = NpcMood::Calm;
    NpcRoutinePhase routine = NpcRoutinePhase::IdleHome;

    std::uint32_t activeAffordanceId = 0;
    std::uint32_t activeSeatAnchorId = 0;
    bool seatOccupancy = false;

    float activitySeconds = 0.0f;
    float playerReactionCooldown = 0.0f;
    std::uint64_t simulationTicks = 0;
    std::uint32_t routineCycles = 0;

    NpcNeeds needs{};
};

} // namespace hakui
