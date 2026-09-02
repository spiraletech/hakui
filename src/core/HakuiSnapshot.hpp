#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "core/GameRuntime.hpp"

namespace hakui {

// L6 versioned, read-only truth packet.
//
// Snapshot values are owned copies: no pointer/reference into GameRuntime is
// exposed. This is the contract later consumed by HakuiAdapter/Spiral and by
// external observers without granting mutation authority over simulation.
struct HakuiAnchorSnapshot {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float yaw = 0.0f;
};

struct HakuiAffordanceSnapshot {
    std::uint32_t id = 0;
    std::string label;
    WorldAffordanceMask affordances = 0;

    float minimumX = 0.0f;
    float maximumX = 0.0f;
    float minimumY = 0.0f;
    float maximumY = 0.0f;
    float minimumZ = 0.0f;
    float maximumZ = 0.0f;

    HakuiAnchorSnapshot primaryAnchor;
    HakuiAnchorSnapshot secondaryAnchor;
};

struct HakuiSeatSnapshot {
    std::uint32_t id = 0;
    std::uint32_t furnitureAffordanceId = 0;
    std::string label;
    HakuiAnchorSnapshot worldPosition;
    bool occupied = false;
    SeatPoseProfile poseProfile = SeatPoseProfile::LoungeRelaxed;
};

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

    // Semantic world truth only. Raw render geometry is deliberately omitted;
    // L7 can reason about usable world affordances without becoming coupled to
    // presentation primitives.
    std::vector<HakuiAffordanceSnapshot> affordances;
    std::vector<HakuiSeatSnapshot> seats;
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

[[nodiscard]] constexpr HakuiAnchorSnapshot snapshotAnchor(
    const WorldAnchor& anchor
) noexcept
{
    return {anchor.x, anchor.y, anchor.z, anchor.yaw};
}

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

    snapshot.world.affordances.reserve(room.affordances().size());
    for (const WorldAffordanceVolume& volume : room.affordances()) {
        HakuiAffordanceSnapshot affordance;
        affordance.id = volume.id;
        affordance.label = std::string(volume.label);
        affordance.affordances = volume.affordances;
        affordance.minimumX = volume.minimumX;
        affordance.maximumX = volume.maximumX;
        affordance.minimumY = volume.minimumY;
        affordance.maximumY = volume.maximumY;
        affordance.minimumZ = volume.minimumZ;
        affordance.maximumZ = volume.maximumZ;
        affordance.primaryAnchor = snapshotAnchor(volume.primaryAnchor);
        affordance.secondaryAnchor = snapshotAnchor(volume.secondaryAnchor);
        snapshot.world.affordances.push_back(std::move(affordance));
    }

    snapshot.world.seats.reserve(room.seatAnchors().size());
    for (const SeatAnchor& seat : room.seatAnchors()) {
        const ResolvedSeatAnchor resolved = room.resolvedSeatAnchor(seat.id);

        HakuiSeatSnapshot seatSnapshot;
        seatSnapshot.id = seat.id;
        seatSnapshot.furnitureAffordanceId = seat.furnitureAffordanceId;
        seatSnapshot.label = std::string(seat.label);
        seatSnapshot.worldPosition = snapshotAnchor(resolved.worldPosition);
        seatSnapshot.occupied = seat.occupied;
        seatSnapshot.poseProfile = seat.poseProfile;
        snapshot.world.seats.push_back(std::move(seatSnapshot));

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
