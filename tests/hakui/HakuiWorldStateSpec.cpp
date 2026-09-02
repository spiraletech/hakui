#include "world/HakuiWorldState.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <string_view>

namespace {

bool near(float left, float right, float epsilon = 0.0001f) noexcept
{
    return std::fabs(left - right) <= epsilon;
}

} // namespace

int main()
{
    hakui::HakuiWorldState world;

    static_assert(hakui::HakuiWorldState::schemaVersion == 1);
    static_assert(
        hakui::HakuiWorldState::canonicalScene ==
        hakui::HakuiWorldScene::BlackRoom
    );
    assert(
        hakui::HakuiWorldState::canonicalWorldId ==
        std::string_view{"hakui.black_room"}
    );

    // The authored proof room is owned by the world state and keeps one stable
    // address for the lifetime of that authority boundary.
    hakui::BlackRoom* const roomAddress = &world.blackRoom();
    assert(&world.blackRoom() == roomAddress);
    const hakui::MovementEnvironment environment =
        world.blackRoom().movementEnvironment();
    assert(environment.hasFloorAt(environment.spawnX, environment.spawnZ));

    // Simulation time is deterministic, monotonic for valid positive deltas,
    // and counts accepted simulation steps.
    assert(near(world.clock().seconds(), 0.0f));
    assert(world.clock().step() == 0);

    world.advance(0.25f);
    assert(near(world.clock().seconds(), 0.25f));
    assert(world.clock().step() == 1);

    // L3 compatibility syntax remains wired to the same canonical clock.
    world.elapsedSeconds += 0.5f;
    assert(near(static_cast<float>(world.elapsedSeconds), 0.75f));
    assert(world.clock().step() == 2);

    world.advance(0.0f);
    world.advance(-1.0f);
    world.advance(std::numeric_limits<float>::quiet_NaN());
    assert(near(world.clock().seconds(), 0.75f));
    assert(world.clock().step() == 2);

    // Mutable authored-room state belongs to this world authority as well.
    const hakui::WorldAffordanceVolume* couch =
        world.blackRoom().affordanceById(1002);
    assert(couch != nullptr);

    PlayerState player;
    player.x = couch->secondaryAnchor.x;
    player.y = couch->secondaryAnchor.y;
    player.z = couch->secondaryAnchor.z;
    player.yaw = couch->secondaryAnchor.yaw;
    player.grounded = true;
    player.activity = PlayerActivity::Roaming;

    assert(world.blackRoom().engageNearest(player));
    const std::uint32_t occupiedSeat = player.activeSeatAnchorId;
    assert(occupiedSeat != 0);
    assert(world.blackRoom().seatOccupied(occupiedSeat));

    // A full world reset clears clock and mutable room state without changing
    // the address of the canonical world/room authority.
    world.reset();
    assert(&world.blackRoom() == roomAddress);
    assert(near(world.clock().seconds(), 0.0f));
    assert(world.clock().step() == 0);
    assert(!world.blackRoom().seatOccupied(occupiedSeat));

    return 0;
}
