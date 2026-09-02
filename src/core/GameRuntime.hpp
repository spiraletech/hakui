#pragma once

#include "interaction/InteractionRegistry.hpp"
#include "player/PlayerRuntime.hpp"
#include "world/HakuiWorldState.hpp"

namespace hakui {

// L5 deterministic authority root.
//
// GameRuntime no longer stores player controllers/state as loose siblings.
// World, player and interaction membership each have one explicit owner while
// platform-bound routing, SDL, rendering, audio, chat, combat and Spiral
// orchestration stay outside this class.
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

    InteractionRegistry& interactionRegistry() noexcept { return interactions_; }
    const InteractionRegistry& interactionRegistry() const noexcept { return interactions_; }

    // Compatibility accessors for the existing native-client call sites.
    // These delegate into the explicit L5 authority roots rather than exposing
    // duplicate state.
    BlackRoom& blackRoom() noexcept { return world_.blackRoom(); }
    const BlackRoom& blackRoom() const noexcept { return world_.blackRoom(); }

    PlayerState& player() noexcept { return player_.state(); }
    const PlayerState& player() const noexcept { return player_.state(); }

    PlayerMovementController& movement() noexcept { return player_.movement(); }
    const PlayerMovementController& movement() const noexcept { return player_.movement(); }

    RideableMovementController& rideable() noexcept { return player_.rideable(); }
    const RideableMovementController& rideable() const noexcept { return player_.rideable(); }

    void advanceWorld(float deltaSeconds) noexcept
    {
        world_.advance(deltaSeconds);
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
    }

private:
    HakuiWorldState world_{};
    PlayerRuntime player_{};
    InteractionRegistry interactions_{};
};

} // namespace hakui
