#pragma once

#include "player/PlayerMovementController.hpp"
#include "player/PlayerState.hpp"
#include "player/RideableMovementController.hpp"

namespace hakui {

// L5 authoritative player aggregate.
//
// Player state and the deterministic controllers that mutate/interpret it live
// behind one stable owner. Platform input, camera, rendering, audio, combat and
// social presentation remain outside this type.
class PlayerRuntime final {
public:
    PlayerRuntime() = default;
    PlayerRuntime(const PlayerRuntime&) = delete;
    PlayerRuntime& operator=(const PlayerRuntime&) = delete;
    PlayerRuntime(PlayerRuntime&&) = delete;
    PlayerRuntime& operator=(PlayerRuntime&&) = delete;

    PlayerState& state() noexcept { return state_; }
    const PlayerState& state() const noexcept { return state_; }

    PlayerMovementController& movement() noexcept { return movement_; }
    const PlayerMovementController& movement() const noexcept { return movement_; }

    RideableMovementController& rideable() noexcept { return rideable_; }
    const RideableMovementController& rideable() const noexcept { return rideable_; }

    void resetToSpawn(
        const MovementEnvironment& environment,
        float startingMoney = 250.0f
    ) noexcept
    {
        state_ = PlayerState{};
        state_.x = environment.spawnX;
        state_.y = environment.spawnY;
        state_.z = environment.spawnZ;
        state_.money = startingMoney;
        rideable_.reset();
    }

private:
    PlayerState state_{};
    PlayerMovementController movement_{};
    RideableMovementController rideable_{};
};

} // namespace hakui
