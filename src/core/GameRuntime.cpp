#include "core/GameRuntime.hpp"

namespace hakui {

GameRuntime::GameRuntime()
{
    prepareNewSession();
}

void GameRuntime::prepareNewSession(float startingMoney) noexcept
{
    world_ = {};
    blackRoom_ = {};
    combat_ = combat::CombatSimulation{};
    chat_ = social::ChatSystem{};
    player_ = {};
    movement_ = PlayerMovementController{};
    rideable_ = RideableMovementController{};

    const MovementEnvironment environment = blackRoom_.movementEnvironment();
    player_.x = environment.spawnX;
    player_.y = environment.spawnY;
    player_.z = environment.spawnZ;
    player_.money = startingMoney;
}

::WorldState& GameRuntime::world() noexcept
{
    return world_;
}

const ::WorldState& GameRuntime::world() const noexcept
{
    return world_;
}

BlackRoom& GameRuntime::blackRoom() noexcept
{
    return blackRoom_;
}

const BlackRoom& GameRuntime::blackRoom() const noexcept
{
    return blackRoom_;
}

combat::CombatSimulation& GameRuntime::combat() noexcept
{
    return combat_;
}

const combat::CombatSimulation& GameRuntime::combat() const noexcept
{
    return combat_;
}

social::ChatSystem& GameRuntime::chat() noexcept
{
    return chat_;
}

const social::ChatSystem& GameRuntime::chat() const noexcept
{
    return chat_;
}

::PlayerState& GameRuntime::player() noexcept
{
    return player_;
}

const ::PlayerState& GameRuntime::player() const noexcept
{
    return player_;
}

PlayerMovementController& GameRuntime::movement() noexcept
{
    return movement_;
}

const PlayerMovementController& GameRuntime::movement() const noexcept
{
    return movement_;
}

RideableMovementController& GameRuntime::rideable() noexcept
{
    return rideable_;
}

const RideableMovementController& GameRuntime::rideable() const noexcept
{
    return rideable_;
}

} // namespace hakui
