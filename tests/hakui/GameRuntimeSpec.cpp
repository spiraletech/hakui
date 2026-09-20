#include "core/GameRuntime.hpp"

#include <cassert>
#include <cmath>

namespace {

bool near(float a, float b, float epsilon = 0.0001f)
{
    return std::fabs(a - b) <= epsilon;
}

} // namespace

int main()
{
    hakui::GameRuntime runtime;

    const hakui::MovementEnvironment environment =
        runtime.blackRoom().movementEnvironment();

    assert(near(runtime.world().elapsedSeconds, 0.0f));
    assert(near(runtime.player().x, environment.spawnX));
    assert(near(runtime.player().y, environment.spawnY));
    assert(near(runtime.player().z, environment.spawnZ));
    assert(near(runtime.player().money, 250.0f));
    assert(runtime.player().activity == PlayerActivity::Roaming);
    assert(!runtime.combat().active());
    assert(runtime.chat().history().empty());

    // Prove the runtime owns one shared authoritative player state: controller
    // writes are visible through the same object returned by player().
    hakui::MovementInput input;
    input.forward = 1.0f;
    const float previousZ = runtime.player().z;
    (void)runtime.movement().update(
        runtime.player(),
        input,
        environment,
        1.0f / 60.0f
    );
    assert(runtime.player().z != previousZ || runtime.player().velocityZ != 0.0f);

    // Exercise mutable room state, then prove a new session restores both the
    // player and furniture occupancy instead of leaking state through HakuiApp.
    const hakui::WorldAffordanceVolume* couch =
        runtime.blackRoom().affordanceById(1002);
    assert(couch != nullptr);

    runtime.player().x = couch->secondaryAnchor.x;
    runtime.player().y = couch->secondaryAnchor.y;
    runtime.player().z = couch->secondaryAnchor.z;
    runtime.player().grounded = true;
    runtime.player().activity = PlayerActivity::Roaming;

    assert(runtime.blackRoom().engageNearest(runtime.player()));
    assert(runtime.player().activity == PlayerActivity::CouchSeated);
    assert(runtime.player().seatOccupancy);

    const std::uint32_t occupiedSeat = runtime.player().activeSeatAnchorId;
    assert(occupiedSeat != 0);
    assert(runtime.blackRoom().seatOccupied(occupiedSeat));

    runtime.world().elapsedSeconds = 42.0f;
    runtime.prepareNewSession(125.0f);

    const hakui::MovementEnvironment resetEnvironment =
        runtime.blackRoom().movementEnvironment();
    assert(near(runtime.world().elapsedSeconds, 0.0f));
    assert(near(runtime.player().x, resetEnvironment.spawnX));
    assert(near(runtime.player().y, resetEnvironment.spawnY));
    assert(near(runtime.player().z, resetEnvironment.spawnZ));
    assert(near(runtime.player().money, 125.0f));
    assert(runtime.player().activity == PlayerActivity::Roaming);
    assert(!runtime.player().seatOccupancy);
    assert(!runtime.blackRoom().seatOccupied(100201));
    assert(!runtime.blackRoom().seatOccupied(100202));
    assert(!runtime.combat().active());
    assert(runtime.chat().history().empty());

    return 0;
}
