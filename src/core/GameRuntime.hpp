#pragma once

#include "player/PlayerMovementController.hpp"
#include "player/RideableMovementController.hpp"
#include "player/PlayerState.hpp"
#include "world/HakuiWorldState.hpp"

namespace hakui {

// L3 runtime ownership boundary, strengthened by L4 canonical world authority.
//
// GameRuntime owns deterministic player/movement state plus exactly one
// HakuiWorldState. Platform-bound routing, SDL, rendering, audio, chat, combat
// and Spiral orchestration intentionally remain outside this class.
class GameRuntime final {
public:
    GameRuntime() = default;
    GameRuntime(const GameRuntime&) = delete;
    GameRuntime& operator=(const GameRuntime&) = delete;
    GameRuntime(GameRuntime&&) = delete;
    GameRuntime& operator=(GameRuntime&&) = delete;

    HakuiWorldState& world() noexcept { return world_; }
    const HakuiWorldState& world() const noexcept { return world_; }

    // Compatibility accessor for L3/native-client call sites. BlackRoom is no
    // longer a sibling authority; it is owned by the canonical world state.
    BlackRoom& blackRoom() noexcept { return world_.blackRoom(); }
    const BlackRoom& blackRoom() const noexcept { return world_.blackRoom(); }

    PlayerState& player() noexcept { return player_; }
    const PlayerState& player() const noexcept { return player_; }

    PlayerMovementController& movement() noexcept { return movement_; }
    const PlayerMovementController& movement() const noexcept { return movement_; }

    RideableMovementController& rideable() noexcept { return rideable_; }
    const RideableMovementController& rideable() const noexcept { return rideable_; }

    void advanceWorld(float deltaSeconds) noexcept
    {
        world_.advance(deltaSeconds);
    }

    // Establish a fresh deterministic player session at the authored room
    // spawn without resetting unrelated mutable world state.
    void resetPlayerToSpawn(float startingMoney = 250.0f) noexcept
    {
        const MovementEnvironment room = world_.blackRoom().movementEnvironment();
        player_ = PlayerState{};
        player_.x = room.spawnX;
        player_.y = room.spawnY;
        player_.z = room.spawnZ;
        player_.money = startingMoney;
        rideable_.reset();
    }

    // Full deterministic session reset: world clock + mutable authored-room
    // state + player/ride state all return to their canonical defaults.
    void resetSession(float startingMoney = 250.0f) noexcept
    {
        world_.reset();
        resetPlayerToSpawn(startingMoney);
    }

private:
    HakuiWorldState world_{};
    PlayerState player_{};
    PlayerMovementController movement_{};
    RideableMovementController rideable_{};
};

} // namespace hakui
