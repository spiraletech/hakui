#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "npc/NpcState.hpp"
#include "player/PlayerState.hpp"
#include "world/BlackRoom.hpp"

namespace hakui {

// L10 deterministic NPC authority. The manager owns resident simulation state
// and consumes only first-party world/player truth. It has no SDL, renderer,
// Spiral, model, network, audio or persistence dependency.
class NpcManager final {
public:
    static constexpr std::uint32_t saelisId = 2001;

    NpcManager();

    NpcManager(const NpcManager&) = delete;
    NpcManager& operator=(const NpcManager&) = delete;
    NpcManager(NpcManager&&) = delete;
    NpcManager& operator=(NpcManager&&) = delete;

    [[nodiscard]] std::span<NpcState> states() noexcept
    {
        return {npcs_.data(), npcs_.size()};
    }

    [[nodiscard]] std::span<const NpcState> states() const noexcept
    {
        return {npcs_.data(), npcs_.size()};
    }

    [[nodiscard]] std::size_t size() const noexcept { return npcs_.size(); }

    [[nodiscard]] NpcState* find(std::uint32_t id) noexcept;
    [[nodiscard]] const NpcState* find(std::uint32_t id) const noexcept;

    // Narrow mutation verbs used by L11's permission gate. The manager remains
    // the resident authority and may reject a command that conflicts with
    // physical state (for example, interrupting an occupied couch seat).
    bool requestObservePlayer(std::uint32_t id, const PlayerState& player) noexcept;
    bool requestResumeRoutine(std::uint32_t id) noexcept;
    bool requestWalkToPlayer(
        std::uint32_t id,
        const PlayerState& player,
        const BlackRoom& room
    );

    // Reset resident state to authored spawn/routine defaults. Any seat still
    // owned by a resident is released first so player/NPC furniture truth stays
    // coherent across deterministic session resets.
    void reset(BlackRoom& room) noexcept;

    // Advance all residents from authoritative HAKUI state. Invalid/non-positive
    // deltas are ignored and never mutate NPC state.
    void tick(
        BlackRoom& room,
        const PlayerState& player,
        float deltaSeconds
    ) noexcept;

private:
    static NpcState makeSaelis() noexcept;
    static void setTarget(
        NpcState& npc,
        float x,
        float y,
        float z
    ) noexcept;
    static bool moveTowardTarget(NpcState& npc, float deltaSeconds) noexcept;
    static void facePoint(NpcState& npc, float x, float z) noexcept;
    static void updateLocomotionPose(NpcState& npc, float deltaSeconds) noexcept;
    static void updateNeeds(NpcState& npc, float deltaSeconds) noexcept;
    static bool reserveCouchSeat(BlackRoom& room, NpcState& npc) noexcept;
    static void releaseSeat(BlackRoom& room, NpcState& npc) noexcept;
    static void resumeRoutineActivity(NpcState& npc) noexcept;

    void tickNpc(
        NpcState& npc,
        BlackRoom& room,
        const PlayerState& player,
        float deltaSeconds
    ) noexcept;

    std::vector<NpcState> npcs_;
};

} // namespace hakui
