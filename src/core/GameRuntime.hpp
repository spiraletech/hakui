#pragma once

#include "combat/CombatSimulation.hpp"
#include "player/PlayerMovementController.hpp"
#include "player/RideableMovementController.hpp"
#include "player/PlayerState.hpp"
#include "social/ChatSystem.hpp"
#include "world/BlackRoom.hpp"
#include "world/WorldState.hpp"

namespace hakui {

// GameRuntime is the deterministic live-game ownership boundary.
//
// Platform input, SDL, GPU rendering, audio, native window state, and Spiral
// transport stay outside this class. The runtime owns authoritative local game
// state and the deterministic controllers that mutate that state. This lets the
// native HakuiApp become orchestration instead of the place where gameplay
// lifetime is defined.
class GameRuntime final {
public:
    GameRuntime();

    // Restore a clean local session without recreating the platform shell.
    // This is deliberately deterministic and side-effect free outside HAKUI
    // state; persistence/network bootstrapping can replace it later.
    void prepareNewSession(float startingMoney = 250.0f) noexcept;

    ::WorldState& world() noexcept;
    const ::WorldState& world() const noexcept;

    BlackRoom& blackRoom() noexcept;
    const BlackRoom& blackRoom() const noexcept;

    combat::CombatSimulation& combat() noexcept;
    const combat::CombatSimulation& combat() const noexcept;

    social::ChatSystem& chat() noexcept;
    const social::ChatSystem& chat() const noexcept;

    ::PlayerState& player() noexcept;
    const ::PlayerState& player() const noexcept;

    PlayerMovementController& movement() noexcept;
    const PlayerMovementController& movement() const noexcept;

    RideableMovementController& rideable() noexcept;
    const RideableMovementController& rideable() const noexcept;

private:
    ::WorldState world_{};
    BlackRoom blackRoom_{};
    combat::CombatSimulation combat_{};
    social::ChatSystem chat_{};
    ::PlayerState player_{};
    PlayerMovementController movement_{};
    RideableMovementController rideable_{};
};

} // namespace hakui
