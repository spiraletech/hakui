#pragma once

#include "action/HakuiActionGate.hpp"
#include "interaction/InteractionRegistry.hpp"
#include "npc/NpcManager.hpp"
#include "player/PlayerRuntime.hpp"
#include "world/HakuiWorldState.hpp"

namespace hakui {

// Deterministic authority root.
//
// L10 extends the L5 split with an explicit NPC authority. World, player,
// residents and interaction membership each have one owner while platform
// input, rendering, audio, chat, combat and Spiral orchestration remain outside
// this class.
class GameRuntime final {
public:
    GameRuntime() = default;
    GameRuntime(const GameRuntime&) = delete;
    GameRuntime& operator=(const GameRuntime&) = delete;
    GameRuntime(GameRuntime&&) = delete;
    GameRuntime& operator=(GameRuntime&&) = delete;

    HakuiWorldState& world() noexcept { return world_; }
    const HakuiWorldState& world() const noexcept { return world_; }

    PlayerRuntime& playerRuntime() noexcept { return player_; }
    const PlayerRuntime& playerRuntime() const noexcept { return player_; }

    NpcManager& npcs() noexcept { return npcs_; }
    const NpcManager& npcs() const noexcept { return npcs_; }

    HakuiActionGate& actionGate() noexcept { return actionGate_; }
    const HakuiActionGate& actionGate() const noexcept { return actionGate_; }

    InteractionRegistry& interactionRegistry() noexcept { return interactions_; }
    const InteractionRegistry& interactionRegistry() const noexcept { return interactions_; }

    // Compatibility accessors for the existing native-client call sites.
    // These delegate into explicit authority roots rather than exposing
    // duplicate state.
    BlackRoom& blackRoom() noexcept { return world_.blackRoom(); }
    const BlackRoom& blackRoom() const noexcept { return world_.blackRoom(); }

    PlayerState& player() noexcept { return player_.state(); }
    const PlayerState& player() const noexcept { return player_.state(); }

    PlayerMovementController& movement() noexcept { return player_.movement(); }
    const PlayerMovementController& movement() const noexcept { return player_.movement(); }

    RideableMovementController& rideable() noexcept { return player_.rideable(); }
    const RideableMovementController& rideable() const noexcept { return player_.rideable(); }

    // Advance one accepted deterministic simulation delta. NPCs tick only when
    // the canonical world clock accepts the delta, so invalid/overflow deltas
    // cannot mutate resident state while leaving world time unchanged.
    void advanceWorld(float deltaSeconds) noexcept
    {
        const std::uint64_t beforeStep = world_.clock().step();
        world_.advance(deltaSeconds);
        if (world_.clock().step() != beforeStep) {
            npcs_.tick(world_.blackRoom(), player_.state(), deltaSeconds);
        }
    }

    // Reset only player/ride state against the current authored world.
    void resetPlayerToSpawn(float startingMoney = 250.0f) noexcept
    {
        player_.resetToSpawn(world_.blackRoom().movementEnvironment(), startingMoney);
    }

    // Full deterministic gameplay reset. Interaction membership intentionally
    // remains separate: live world objects keep their registered endpoints
    // unless their owner explicitly unregisters or destroys them.
    void resetSession(float startingMoney = 250.0f) noexcept
    {
        world_.reset();
        resetPlayerToSpawn(startingMoney);
        npcs_.reset(world_.blackRoom());
    }

private:
    HakuiWorldState world_{};
    PlayerRuntime player_{};
    NpcManager npcs_{};
    HakuiActionGate actionGate_{};
    InteractionRegistry interactions_{};
};

} // namespace hakui
