#pragma once

#include "player/PlayerMovementController.hpp"
#include "player/RideableMovementController.hpp"
#include "player/PlayerState.hpp"
#include "systems/LocomotionRouter.hpp"
#include "world/BlackRoom.hpp"
#include "world/WorldState.hpp"

namespace hakui {

// L3 runtime ownership boundary.
//
// GameRuntime owns deterministic world/player/locomotion state that used to
// live directly on HakuiApp. SDL, rendering, audio, chat, combat and Spiral
// orchestration intentionally remain outside this class for now; later layers
// can migrate across this boundary without changing the platform shell.
class GameRuntime final {
public:
    GameRuntime() = default;
    GameRuntime(const GameRuntime&) = delete;
    GameRuntime& operator=(const GameRuntime&) = delete;
    GameRuntime(GameRuntime&&) = delete;
    GameRuntime& operator=(GameRuntime&&) = delete;

    WorldState& world() noexcept { return world_; }
    const WorldState& world() const noexcept { return world_; }

    BlackRoom& blackRoom() noexcept { return blackRoom_; }
    const BlackRoom& blackRoom() const noexcept { return blackRoom_; }

    PlayerState& player() noexcept { return player_; }
    const PlayerState& player() const noexcept { return player_; }

    PlayerMovementController& movement() noexcept { return movement_; }
    const PlayerMovementController& movement() const noexcept { return movement_; }

    RideableMovementController& rideable() noexcept { return rideable_; }
    const RideableMovementController& rideable() const noexcept { return rideable_; }

    LocomotionRouter& locomotion() noexcept { return locomotion_; }
    const LocomotionRouter& locomotion() const noexcept { return locomotion_; }

    // Establish a fresh deterministic player session at the authored room
    // spawn. Existing HakuiApp boot code can migrate to this call separately;
    // ownership is extracted first so L3 remains behavior-preserving.
    void resetPlayerToSpawn(float startingMoney = 250.0f) noexcept
    {
        const MovementEnvironment room = blackRoom_.movementEnvironment();
        player_ = PlayerState{};
        player_.x = room.spawnX;
        player_.y = room.spawnY;
        player_.z = room.spawnZ;
        player_.money = startingMoney;
        rideable_.reset();
    }

private:
    WorldState world_{};
    BlackRoom blackRoom_{};
    PlayerState player_{};
    PlayerMovementController movement_{};
    RideableMovementController rideable_{};
    LocomotionRouter locomotion_{player_};
};

} // namespace hakui
