#include "world/BlackRoom.hpp"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <unordered_set>

namespace {

constexpr float kEpsilon = 0.001f;

bool finite(float value) noexcept
{
    return std::isfinite(value);
}

bool nearlyEqual(float left, float right, float epsilon = kEpsilon) noexcept
{
    return std::fabs(left - right) <= epsilon;
}

struct PrimitiveBounds {
    float minimumX;
    float maximumX;
    float minimumY;
    float maximumY;
    float minimumZ;
    float maximumZ;
};

PrimitiveBounds boundsForInstance(
    const hakui::WorldPrimitive& primitive,
    std::uint16_t instance
) noexcept
{
    const float x = primitive.x + primitive.repeatX * static_cast<float>(instance);
    const float y = primitive.y + primitive.repeatY * static_cast<float>(instance);
    const float z = primitive.z + primitive.repeatZ * static_cast<float>(instance);

    return {
        x - primitive.width * 0.5f,
        x + primitive.width * 0.5f,
        y - primitive.height * 0.5f,
        y + primitive.height * 0.5f,
        z - primitive.depth * 0.5f,
        z + primitive.depth * 0.5f
    };
}

bool containsXZ(
    const hakui::HorizontalCollider& collider,
    float x,
    float z
) noexcept
{
    return x >= collider.minimumX && x <= collider.maximumX &&
           z >= collider.minimumZ && z <= collider.maximumZ;
}

} // namespace

int main()
{
    using hakui::BlackRoom;
    using hakui::RoomInteractionKind;
    using hakui::WorldAffordance;
    using hakui::WorldPrimitiveKind;

    BlackRoom room;

    // L2 invariant 1: authored world primitives must remain finite, positive,
    // and bounded. Repeat fields are geometry instancing, so validate every
    // expanded instance rather than only the source primitive.
    std::size_t expandedPrimitiveCount = 0;
    for (const hakui::WorldPrimitive& primitive : room.geometry()) {
        assert(finite(primitive.x));
        assert(finite(primitive.y));
        assert(finite(primitive.z));
        assert(finite(primitive.width));
        assert(finite(primitive.height));
        assert(finite(primitive.depth));
        assert(finite(primitive.rotationX));
        assert(finite(primitive.rotationY));
        assert(finite(primitive.rotationZ));
        assert(finite(primitive.repeatX));
        assert(finite(primitive.repeatY));
        assert(finite(primitive.repeatZ));
        assert(primitive.width > 0.0f);
        assert(primitive.height > 0.0f);
        assert(primitive.depth > 0.0f);
        assert(primitive.repeatCount > 0);

        for (std::uint16_t instance = 0; instance < primitive.repeatCount; ++instance) {
            const PrimitiveBounds bounds = boundsForInstance(primitive, instance);
            assert(finite(bounds.minimumX));
            assert(finite(bounds.maximumX));
            assert(finite(bounds.minimumY));
            assert(finite(bounds.maximumY));
            assert(finite(bounds.minimumZ));
            assert(finite(bounds.maximumZ));
            assert(bounds.minimumX <= bounds.maximumX);
            assert(bounds.minimumY <= bounds.maximumY);
            assert(bounds.minimumZ <= bounds.maximumZ);
            ++expandedPrimitiveCount;
        }
    }
    assert(expandedPrimitiveCount > 0);
    assert(expandedPrimitiveCount < 256);

    // L2 invariant 2: affordance identifiers are unique and every volume has a
    // sane axis-aligned extent. These IDs are gameplay contracts, not visuals.
    std::unordered_set<std::uint32_t> affordanceIds;
    for (const hakui::WorldAffordanceVolume& volume : room.affordances()) {
        assert(volume.id != 0);
        assert(affordanceIds.insert(volume.id).second);
        assert(volume.minimumX <= volume.maximumX);
        assert(volume.minimumY <= volume.maximumY);
        assert(volume.minimumZ <= volume.maximumZ);
        assert(finite(volume.primaryAnchor.x));
        assert(finite(volume.primaryAnchor.y));
        assert(finite(volume.primaryAnchor.z));
        assert(finite(volume.primaryAnchor.yaw));
        assert(finite(volume.secondaryAnchor.x));
        assert(finite(volume.secondaryAnchor.y));
        assert(finite(volume.secondaryAnchor.z));
        assert(finite(volume.secondaryAnchor.yaw));
    }

    // L2 invariant 3: the runtime spawn must remain on the authored floor and
    // inside the recovery affordance so a void reset cannot strand the player.
    const hakui::MovementEnvironment environment = room.movementEnvironment();
    assert(environment.hasFloorAt(environment.spawnX, environment.spawnZ));
    const hakui::WorldAffordanceVolume* recovery = room.affordanceById(1301);
    assert(recovery != nullptr);
    assert(hakui::hasAffordance(recovery->affordances, WorldAffordance::RespawnVolume));
    assert(recovery->contains(
        environment.spawnX,
        environment.spawnY,
        environment.spawnZ
    ));

    // L2 invariant 4: the Void Couch is exactly the intended two-seat visual.
    // A duplicated repeated cushion primitive previously stretched the couch
    // far to the right. Lock its visual envelope and expanded instance count.
    const hakui::WorldAffordanceVolume* couch = room.affordanceById(1002);
    assert(couch != nullptr);
    assert(hakui::hasAffordance(couch->affordances, WorldAffordance::Seat));
    assert(!hakui::hasAffordance(couch->affordances, WorldAffordance::CasinoAnchor));

    constexpr float couchMinimumX = 4.00f;
    constexpr float couchMaximumX = 7.40f;
    constexpr float couchMinimumZ = 3.80f;
    constexpr float couchMaximumZ = 5.45f;
    std::size_t couchFurnitureInstances = 0;

    for (const hakui::WorldPrimitive& primitive : room.geometry()) {
        if (primitive.kind != WorldPrimitiveKind::Furniture) {
            continue;
        }

        for (std::uint16_t instance = 0; instance < primitive.repeatCount; ++instance) {
            const float instanceX = primitive.x + primitive.repeatX * static_cast<float>(instance);
            const float instanceZ = primitive.z + primitive.repeatZ * static_cast<float>(instance);
            if (instanceX < couchMinimumX || instanceX > couchMaximumX ||
                instanceZ < couchMinimumZ || instanceZ > couchMaximumZ) {
                continue;
            }

            const PrimitiveBounds bounds = boundsForInstance(primitive, instance);
            assert(bounds.minimumX >= couchMinimumX - kEpsilon);
            assert(bounds.maximumX <= couchMaximumX + kEpsilon);
            assert(bounds.minimumZ >= couchMinimumZ - kEpsilon);
            assert(bounds.maximumZ <= couchMaximumZ + kEpsilon);
            ++couchFurnitureInstances;
        }
    }
    assert(couchFurnitureInstances == 6);

    // The movement collider must cover the couch body without reproducing the
    // old extended tail.
    const hakui::HorizontalCollider* couchCollider = nullptr;
    for (const hakui::HorizontalCollider& collider : environment.colliders) {
        if (containsXZ(collider, couch->primaryAnchor.x, couch->primaryAnchor.z)) {
            couchCollider = &collider;
            break;
        }
    }
    assert(couchCollider != nullptr);
    assert(couchCollider->minimumX >= couchMinimumX - kEpsilon);
    assert(couchCollider->maximumX <= couchMaximumX + kEpsilon);
    assert(couchCollider->minimumZ >= couchMinimumZ - kEpsilon);
    assert(couchCollider->maximumZ <= couchMaximumZ + kEpsilon);

    // L2 invariant 5: couch semantics stay two-seat and both anchors resolve
    // inside the same furniture affordance.
    std::size_t couchSeatCount = 0;
    for (const hakui::SeatAnchor& seat : room.seatAnchors()) {
        if (seat.furnitureAffordanceId != couch->id) {
            continue;
        }
        ++couchSeatCount;
        const hakui::ResolvedSeatAnchor resolved = room.resolvedSeatAnchor(seat.id);
        assert(resolved.id == seat.id);
        assert(resolved.furnitureAffordanceId == couch->id);
        assert(couch->contains(
            resolved.worldPosition.x,
            resolved.worldPosition.y,
            resolved.worldPosition.z
        ));
    }
    assert(couchSeatCount == 2);

    // L2 invariant 6: the full lounge interaction is reversible. Entering a
    // seat must align the player exactly; leaving must release the reservation
    // and return to the authored exit anchor so another player can sit.
    PlayerState player;
    player.x = couch->secondaryAnchor.x;
    player.y = couch->secondaryAnchor.y;
    player.z = couch->secondaryAnchor.z;
    player.yaw = couch->secondaryAnchor.yaw;
    player.grounded = true;
    player.activity = PlayerActivity::Roaming;

    const hakui::RoomInteractionFocus focus = room.nearestInteraction(player);
    assert(focus);
    assert(focus.kind == RoomInteractionKind::LoungeCouch);
    assert(focus.affordanceId == couch->id);

    assert(room.engageNearest(player));
    assert(player.activity == PlayerActivity::CouchSeated);
    assert(player.activeAffordanceId == couch->id);
    assert(player.activeSeatAnchorId != 0);
    assert(player.seatOccupancy);
    assert(room.seatOccupied(player.activeSeatAnchorId));
    assert(nearlyEqual(room.seatAlignmentError(player), 0.0f));

    const std::uint32_t occupiedSeat = player.activeSeatAnchorId;
    assert(room.leaveInteraction(player));
    assert(player.activity == PlayerActivity::Roaming);
    assert(player.activeAffordanceId == 0);
    assert(player.activeSeatAnchorId == 0);
    assert(!player.seatOccupancy);
    assert(!room.seatOccupied(occupiedSeat));
    assert(nearlyEqual(player.x, couch->secondaryAnchor.x));
    assert(nearlyEqual(player.y, couch->secondaryAnchor.y));
    assert(nearlyEqual(player.z, couch->secondaryAnchor.z));

    PlayerState secondPlayer;
    secondPlayer.x = couch->secondaryAnchor.x;
    secondPlayer.y = couch->secondaryAnchor.y;
    secondPlayer.z = couch->secondaryAnchor.z;
    secondPlayer.grounded = true;
    assert(room.engageNearest(secondPlayer));
    assert(secondPlayer.activity == PlayerActivity::CouchSeated);
    assert(room.leaveInteraction(secondPlayer));

    return 0;
}
