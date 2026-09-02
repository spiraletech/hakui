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

    // The runtime owns one stable authoritative player instance. Controllers
    // and routers must mutate that same object rather than shadow copies.
    PlayerState* const playerAddress = &runtime.player();
    assert(&runtime.player() == playerAddress);

    const hakui::MovementEnvironment room =
        runtime.blackRoom().movementEnvironment();

    runtime.resetPlayerToSpawn();
    assert(near(runtime.player().x, room.spawnX));
    assert(near(runtime.player().y, room.spawnY));
    assert(near(runtime.player().z, room.spawnZ));
    assert(near(runtime.player().money, 250.0f));
    assert(runtime.player().locomotion == LocomotionMode::OnFoot);

    // Locomotion routing is now runtime-owned and must operate on the exact
    // same player state exposed through GameRuntime.
    runtime.locomotion().switchTo(LocomotionMode::Skateboard);
    assert(runtime.player().locomotion == LocomotionMode::Skateboard);

    runtime.player().x = room.spawnX + 4.0f;
    runtime.player().money = 11.0f;
    runtime.resetPlayerToSpawn(99.0f);
    assert(&runtime.player() == playerAddress);
    assert(near(runtime.player().x, room.spawnX));
    assert(near(runtime.player().z, room.spawnZ));
    assert(near(runtime.player().money, 99.0f));
    assert(runtime.player().locomotion == LocomotionMode::OnFoot);

    // World time is owned by the runtime as well and remains independent from
    // platform/rendering concerns.
    runtime.world().elapsedSeconds = 12.5f;
    assert(near(runtime.world().elapsedSeconds, 12.5f));

    return 0;
}
