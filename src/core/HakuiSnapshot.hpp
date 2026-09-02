#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "core/GameRuntime.hpp"

namespace hakui {

// L6 versioned, read-only truth packet.
//
// Snapshot values are owned copies: no pointer/reference into GameRuntime is
// exposed. This is the contract later consumed by HakuiAdapter/Spiral and by
// external observers without granting mutation authority over simulation.
struct HakuiPlayerSnapshot {
    std::string displayName;
    LocomotionMode locomotion = LocomotionMode::OnFoot;
    PlayerActivity activity = PlayerActivity::Roaming;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float yaw = 0.0f;

    float velocityX = 0.0f;
    float velocityY = 0.0f;
    float velocityZ = 0.0f;
    bool grounded = true;
    bool sprinting = false;

    std::uint32_t activeAffordanceId = 0;
    std::uint32_t activeSeatAnchorId = 0;
    bool seatOccupancy = false;
    float seatAnchorError = 0.0f;
    std::uint32_t voidRespawns = 0;

    float health = 100.0f;
    float hunger = 100.0f;
    float stamina = 100.0f;
    float money = 0.0f;
};

struct HakuiWorldSnapshot {
    std::uint32_t schemaVersion = HakuiWorldState::schemaVersion;
    std::string worldId;
    HakuiWorldScene scene = HakuiWorldScene::BlackRoom;

    float elapsedSeconds = 0.0f;
    std::uint64_t simulationStep = 0;

    std::size_t geometryPrimitiveCount = 0;
    std::size_t affordanceCount = 0;
    std::size_t seatAnchorCount = 0;
    std::size_t occupiedSeatCount = 0;
};

struct HakuiInteractionSnapshot {
    std::vector<EntityId> liveTargetIds;
};

struct HakuiSnapshot {
    static constexpr std::uint32_t schemaVersion = 1;

    std::uint32_t version = schemaVersion;
    HakuiWorldSnapshot world;
    HakuiPlayerSnapshot player;
    HakuiInteractionSnapshot interactions;
};

[[nodiscard]] inline HakuiSnapshot captureHakuiSnapshot(
    const GameRuntime& runtime
)
{
    HakuiSnapshot snapshot;

    const HakuiWorldState& world = runtime.world();
    const BlackRoom& room = world.blackRoom();
    const PlayerState& player = runtime.player();

    snapshot.world.schemaVersion = HakuiWorldState::schemaVersion;
    snapshot.world.worldId = std::string(HakuiWorldState::canonicalWorldId);
    snapshot.world.scene = HakuiWorldState::canonicalScene;
    snapshot.world.elapsedSeconds = world.clock().seconds();
    snapshot.world.simulationStep = world.clock().step();
    snapshot.world.geometryPrimitiveCount = room.geometry().size();
    snapshot.world.affordanceCount = room.affordances().size();
    snapshot.world.seatAnchorCount = room.seatAnchors().size();

    for (const SeatAnchor& seat : room.seatAnchors()) {
        if (seat.occupied) {
            ++snapshot.world.occupiedSeatCount;
        }
    }

    snapshot.player.displayName = player.displayName;
    snapshot.player.locomotion = player.locomotion;
    snapshot.player.activity = player.activity;
    snapshot.player.x = player.x;
    snapshot.player.y = player.y;
    snapshot.player.z = player.z;
    snapshot.player.yaw = player.yaw;
    snapshot.player.velocityX = player.velocityX;
    snapshot.player.velocityY = player.velocityY;
    snapshot.player.velocityZ = player.velocityZ;
    snapshot.player.grounded = player.grounded;
    snapshot.player.sprinting = player.sprinting;
    snapshot.player.activeAffordanceId = player.activeAffordanceId;
    snapshot.player.activeSeatAnchorId = player.activeSeatAnchorId;
    snapshot.player.seatOccupancy = player.seatOccupancy;
    snapshot.player.seatAnchorError = player.seatAnchorError;
    snapshot.player.voidRespawns = player.voidRespawns;
    snapshot.player.health = player.health;
    snapshot.player.hunger = player.hunger;
    snapshot.player.stamina = player.stamina;
    snapshot.player.money = player.money;

    snapshot.interactions.liveTargetIds =
        runtime.interactionRegistry().liveTargetIds();

    return snapshot;
}

} // namespace hakui
