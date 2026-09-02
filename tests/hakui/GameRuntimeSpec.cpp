#include "core/GameRuntime.hpp"

#include <cassert>
#include <cmath>

namespace {

bool near(float a, float b, float epsilon = 0.0001f) noexcept
{
    return std::fabs(a - b) <= epsilon;
}

} // namespace

int main()
{
    hakui::GameRuntime runtime;

    // The runtime owns one stable authoritative player instance. Movement and
    // ride controllers are exposed from the same ownership boundary rather
    // than duplicated on the platform shell.
    PlayerState* const playerAddress = &runtime.player();
    assert(&runtime.player() == playerAddress);
    assert(&runtime.movement() == &runtime.movement());
    assert(&runtime.rideable() == &runtime.rideable());

    // L4: GameRuntime owns exactly one canonical world authority. The legacy
    // BlackRoom accessor must resolve into that world, never a sibling copy.
    hakui::HakuiWorldState* const worldAddress = &runtime.world();
    hakui::BlackRoom* const roomAddress = &runtime.world().blackRoom();
    assert(&runtime.world() == worldAddress);
    assert(&runtime.blackRoom() == roomAddress);
    assert(&runtime.blackRoom() == &runtime.world().blackRoom());

    const hakui::MovementEnvironment room =
        runtime.blackRoom().movementEnvironment();

    runtime.resetPlayerToSpawn();
    assert(near(runtime.player().x, room.spawnX));
    assert(near(runtime.player().y, room.spawnY));
    assert(near(runtime.player().z, room.spawnZ));
    assert(near(runtime.player().money, 250.0f));
    assert(runtime.player().locomotion == LocomotionMode::OnFoot);

    runtime.player().locomotion = LocomotionMode::Skateboard;
    runtime.player().x = room.spawnX + 4.0f;
    runtime.player().money = 11.0f;
    runtime.resetPlayerToSpawn(99.0f);
    assert(&runtime.player() == playerAddress);
    assert(near(runtime.player().x, room.spawnX));
    assert(near(runtime.player().z, room.spawnZ));
    assert(near(runtime.player().money, 99.0f));
    assert(runtime.player().locomotion == LocomotionMode::OnFoot);

    // World time advances through the canonical state and stays independent
    // from platform/rendering concerns.
    runtime.advanceWorld(12.5f);
    assert(near(runtime.world().clock().seconds(), 12.5f));
    assert(runtime.world().clock().step() == 1);

    // Full session reset preserves authority addresses while restoring the
    // deterministic world clock and player spawn defaults.
    runtime.resetSession(333.0f);
    assert(&runtime.world() == worldAddress);
    assert(&runtime.blackRoom() == roomAddress);
    assert(&runtime.player() == playerAddress);
    assert(near(runtime.world().clock().seconds(), 0.0f));
    assert(runtime.world().clock().step() == 0);
    assert(near(runtime.player().x, room.spawnX));
    assert(near(runtime.player().z, room.spawnZ));
    assert(near(runtime.player().money, 333.0f));

    return 0;
}
