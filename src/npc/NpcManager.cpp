#include "npc/NpcManager.hpp"

#include <algorithm>
#include <cmath>

namespace hakui {
namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kSaelisWalkSpeed = 1.15f;
constexpr float kArrivalRadius = 0.08f;
constexpr float kPlayerNoticeRadius = 1.65f;
constexpr float kPlayerNoticeSeconds = 1.25f;
constexpr float kPlayerNoticeCooldown = 3.0f;
constexpr float kCouchRestSeconds = 6.0f;
constexpr float kSpiralObserveSeconds = 4.0f;
constexpr float kHomeIdleSeconds = 3.0f;

float clampNeed(float value) noexcept
{
    return std::clamp(value, 0.0f, 100.0f);
}

float planarDistanceSquared(
    float ax,
    float az,
    float bx,
    float bz
) noexcept
{
    const float dx = bx - ax;
    const float dz = bz - az;
    return dx * dx + dz * dz;
}

bool isWalkingPhase(NpcRoutinePhase phase) noexcept
{
    switch (phase) {
        case NpcRoutinePhase::WalkToCouch:
        case NpcRoutinePhase::WalkSouthBypass:
        case NpcRoutinePhase::WalkToSpiral:
        case NpcRoutinePhase::WalkNorthBypass:
        case NpcRoutinePhase::WalkHome:
            return true;
        case NpcRoutinePhase::CouchRest:
        case NpcRoutinePhase::ObserveSpiral:
        case NpcRoutinePhase::IdleHome:
            return false;
    }
    return false;
}

} // namespace

NpcManager::NpcManager()
{
    npcs_.push_back(makeSaelis());
}

NpcState* NpcManager::find(std::uint32_t id) noexcept
{
    for (NpcState& npc : npcs_) {
        if (npc.id == id) {
            return &npc;
        }
    }
    return nullptr;
}

const NpcState* NpcManager::find(std::uint32_t id) const noexcept
{
    for (const NpcState& npc : npcs_) {
        if (npc.id == id) {
            return &npc;
        }
    }
    return nullptr;
}

NpcState NpcManager::makeSaelis() noexcept
{
    NpcState npc;
    npc.id = saelisId;
    npc.displayName = "SAELIS";

    // Spawn on the left side of the lounge with a clear first route along the
    // north half of the room. The initial couch trip is intentionally long
    // enough that short contract tests cannot accidentally inherit a reserved
    // couch seat simply because simulation time advanced a few seconds.
    npc.x = -4.60f;
    npc.y = 0.0f;
    npc.z = 3.35f;
    npc.yaw = kPi * 0.5f;
    npc.homeX = npc.x;
    npc.homeY = npc.y;
    npc.homeZ = npc.z;

    npc.targetX = 5.72f;
    npc.targetY = 0.0f;
    npc.targetZ = 3.05f;
    npc.activity = NpcActivity::Walking;
    npc.mood = NpcMood::Calm;
    npc.routine = NpcRoutinePhase::WalkToCouch;
    return npc;
}

void NpcManager::setTarget(
    NpcState& npc,
    float x,
    float y,
    float z
) noexcept
{
    npc.targetX = x;
    npc.targetY = y;
    npc.targetZ = z;
}

void NpcManager::facePoint(NpcState& npc, float x, float z) noexcept
{
    const float dx = x - npc.x;
    const float dz = z - npc.z;
    if (std::fabs(dx) <= 0.0001f && std::fabs(dz) <= 0.0001f) {
        return;
    }
    npc.yaw = std::atan2(dx, dz);
}

bool NpcManager::moveTowardTarget(
    NpcState& npc,
    float deltaSeconds
) noexcept
{
    const float dx = npc.targetX - npc.x;
    const float dz = npc.targetZ - npc.z;
    const float distanceSquared = dx * dx + dz * dz;

    if (distanceSquared <= kArrivalRadius * kArrivalRadius) {
        npc.x = npc.targetX;
        npc.y = npc.targetY;
        npc.z = npc.targetZ;
        npc.velocityX = 0.0f;
        npc.velocityZ = 0.0f;
        return true;
    }

    const float distance = std::sqrt(distanceSquared);
    if (!std::isfinite(distance) || distance <= 0.0f) {
        npc.velocityX = 0.0f;
        npc.velocityZ = 0.0f;
        return true;
    }

    const float directionX = dx / distance;
    const float directionZ = dz / distance;
    const float travel = kSaelisWalkSpeed * deltaSeconds;
    npc.yaw = std::atan2(directionX, directionZ);

    if (travel >= distance) {
        npc.x = npc.targetX;
        npc.y = npc.targetY;
        npc.z = npc.targetZ;
        npc.velocityX = 0.0f;
        npc.velocityZ = 0.0f;
        return true;
    }

    npc.velocityX = directionX * kSaelisWalkSpeed;
    npc.velocityZ = directionZ * kSaelisWalkSpeed;
    npc.x += npc.velocityX * deltaSeconds;
    npc.z += npc.velocityZ * deltaSeconds;
    npc.y = npc.targetY;
    return false;
}

void NpcManager::updateNeeds(NpcState& npc, float deltaSeconds) noexcept
{
    npc.needs.hunger = clampNeed(npc.needs.hunger - 0.18f * deltaSeconds);

    const bool seated = npc.activity == NpcActivity::Seated;
    const bool social = npc.activity == NpcActivity::ObservingPlayer;
    const bool curious = npc.activity == NpcActivity::ObservingSpiral;

    npc.needs.energy = clampNeed(
        npc.needs.energy + (seated ? 1.25f : -0.07f) * deltaSeconds
    );
    npc.needs.social = clampNeed(
        npc.needs.social + (social ? 0.80f : -0.025f) * deltaSeconds
    );
    npc.needs.comfort = clampNeed(
        npc.needs.comfort + (seated ? 1.10f : -0.04f) * deltaSeconds
    );
    npc.needs.fun = clampNeed(
        npc.needs.fun + (curious ? 0.35f : -0.018f) * deltaSeconds
    );
}

bool NpcManager::reserveCouchSeat(BlackRoom& room, NpcState& npc) noexcept
{
    constexpr std::uint32_t couchAffordanceId = 1002;

    for (const SeatAnchor& seat : room.seatAnchors()) {
        if (seat.furnitureAffordanceId != couchAffordanceId || seat.occupied) {
            continue;
        }
        if (!room.reserveSeat(seat.id)) {
            continue;
        }

        const ResolvedSeatAnchor resolved = room.resolvedSeatAnchor(seat.id);
        if (resolved.id == 0) {
            (void)room.releaseSeat(seat.id);
            continue;
        }

        npc.activeAffordanceId = couchAffordanceId;
        npc.activeSeatAnchorId = seat.id;
        npc.seatOccupancy = true;
        npc.x = resolved.worldPosition.x;
        npc.y = resolved.worldPosition.y;
        npc.z = resolved.worldPosition.z;
        npc.yaw = resolved.worldPosition.yaw;
        npc.velocityX = 0.0f;
        npc.velocityZ = 0.0f;
        npc.activity = NpcActivity::Seated;
        npc.mood = NpcMood::Resting;
        npc.routine = NpcRoutinePhase::CouchRest;
        npc.activitySeconds = 0.0f;
        return true;
    }

    return false;
}

void NpcManager::releaseSeat(BlackRoom& room, NpcState& npc) noexcept
{
    if (npc.activeSeatAnchorId != 0) {
        (void)room.releaseSeat(npc.activeSeatAnchorId);
    }
    npc.activeAffordanceId = 0;
    npc.activeSeatAnchorId = 0;
    npc.seatOccupancy = false;
}

void NpcManager::resumeRoutineActivity(NpcState& npc) noexcept
{
    if (npc.routine == NpcRoutinePhase::ObserveSpiral) {
        npc.activity = NpcActivity::ObservingSpiral;
        npc.mood = NpcMood::Curious;
    } else if (npc.routine == NpcRoutinePhase::IdleHome) {
        npc.activity = NpcActivity::Idle;
        npc.mood = NpcMood::Calm;
    } else if (isWalkingPhase(npc.routine)) {
        npc.activity = NpcActivity::Walking;
        npc.mood = NpcMood::Calm;
    }
}

void NpcManager::reset(BlackRoom& room) noexcept
{
    for (NpcState& npc : npcs_) {
        releaseSeat(room, npc);
    }
    npcs_.clear();
    npcs_.push_back(makeSaelis());
}

void NpcManager::tick(
    BlackRoom& room,
    const PlayerState& player,
    float deltaSeconds
) noexcept
{
    if (!std::isfinite(deltaSeconds) || deltaSeconds <= 0.0f) {
        return;
    }

    for (NpcState& npc : npcs_) {
        tickNpc(npc, room, player, deltaSeconds);
    }
}

void NpcManager::tickNpc(
    NpcState& npc,
    BlackRoom& room,
    const PlayerState& player,
    float deltaSeconds
) noexcept
{
    ++npc.simulationTicks;
    npc.playerReactionCooldown = std::max(
        0.0f,
        npc.playerReactionCooldown - deltaSeconds
    );
    updateNeeds(npc, deltaSeconds);

    if (npc.activity == NpcActivity::Seated) {
        npc.activitySeconds += deltaSeconds;
        npc.mood = NpcMood::Resting;

        if (npc.activeSeatAnchorId != 0) {
            const ResolvedSeatAnchor seat =
                room.resolvedSeatAnchor(npc.activeSeatAnchorId);
            if (seat.id != 0) {
                npc.x = seat.worldPosition.x;
                npc.y = seat.worldPosition.y;
                npc.z = seat.worldPosition.z;
                npc.yaw = seat.worldPosition.yaw;
            }
        }

        if (npc.activitySeconds >= kCouchRestSeconds) {
            releaseSeat(room, npc);
            npc.activitySeconds = 0.0f;
            npc.activity = NpcActivity::Walking;
            npc.mood = NpcMood::Calm;
            npc.routine = NpcRoutinePhase::WalkSouthBypass;
            setTarget(npc, 5.20f, 0.0f, -2.35f);
        }
        return;
    }

    if (npc.activity == NpcActivity::ObservingPlayer) {
        npc.activitySeconds += deltaSeconds;
        facePoint(npc, player.x, player.z);
        npc.velocityX = 0.0f;
        npc.velocityZ = 0.0f;
        npc.mood = NpcMood::Curious;

        if (npc.activitySeconds >= kPlayerNoticeSeconds) {
            npc.activitySeconds = 0.0f;
            resumeRoutineActivity(npc);
        }
        return;
    }

    if (npc.playerReactionCooldown <= 0.0f &&
        planarDistanceSquared(npc.x, npc.z, player.x, player.z) <=
            kPlayerNoticeRadius * kPlayerNoticeRadius) {
        npc.activity = NpcActivity::ObservingPlayer;
        npc.mood = NpcMood::Curious;
        npc.activitySeconds = 0.0f;
        npc.playerReactionCooldown = kPlayerNoticeCooldown;
        npc.velocityX = 0.0f;
        npc.velocityZ = 0.0f;
        facePoint(npc, player.x, player.z);
        return;
    }

    switch (npc.routine) {
        case NpcRoutinePhase::WalkToCouch: {
            if (const WorldAffordanceVolume* couch = room.affordanceById(1002)) {
                setTarget(
                    npc,
                    couch->secondaryAnchor.x,
                    0.0f,
                    couch->secondaryAnchor.z
                );
            }
            npc.activity = NpcActivity::Walking;
            npc.mood = NpcMood::Calm;
            if (moveTowardTarget(npc, deltaSeconds)) {
                if (!reserveCouchSeat(room, npc)) {
                    npc.routine = NpcRoutinePhase::WalkSouthBypass;
                    setTarget(npc, 5.20f, 0.0f, -2.35f);
                }
            }
            break;
        }

        case NpcRoutinePhase::CouchRest:
            // A valid couch-rest phase always owns a seat and is handled by the
            // early seated branch. Recover safely if state was externally reset.
            npc.routine = NpcRoutinePhase::WalkSouthBypass;
            npc.activity = NpcActivity::Walking;
            setTarget(npc, 5.20f, 0.0f, -2.35f);
            break;

        case NpcRoutinePhase::WalkSouthBypass:
            npc.activity = NpcActivity::Walking;
            npc.mood = NpcMood::Calm;
            if (moveTowardTarget(npc, deltaSeconds)) {
                npc.routine = NpcRoutinePhase::WalkToSpiral;
                if (const WorldAffordanceVolume* spiral = room.affordanceById(1401)) {
                    setTarget(
                        npc,
                        spiral->secondaryAnchor.x,
                        0.0f,
                        spiral->secondaryAnchor.z
                    );
                } else {
                    setTarget(npc, -6.55f, 0.0f, -4.10f);
                }
            }
            break;

        case NpcRoutinePhase::WalkToSpiral:
            npc.activity = NpcActivity::Walking;
            npc.mood = NpcMood::Curious;
            if (moveTowardTarget(npc, deltaSeconds)) {
                npc.routine = NpcRoutinePhase::ObserveSpiral;
                npc.activity = NpcActivity::ObservingSpiral;
                npc.activitySeconds = 0.0f;
                npc.velocityX = 0.0f;
                npc.velocityZ = 0.0f;
                if (const WorldAffordanceVolume* spiral = room.affordanceById(1401)) {
                    facePoint(npc, spiral->primaryAnchor.x, spiral->primaryAnchor.z);
                }
            }
            break;

        case NpcRoutinePhase::ObserveSpiral:
            npc.activity = NpcActivity::ObservingSpiral;
            npc.mood = NpcMood::Curious;
            npc.velocityX = 0.0f;
            npc.velocityZ = 0.0f;
            npc.activitySeconds += deltaSeconds;
            if (npc.activitySeconds >= kSpiralObserveSeconds) {
                npc.activitySeconds = 0.0f;
                npc.routine = NpcRoutinePhase::WalkNorthBypass;
                npc.activity = NpcActivity::Walking;
                npc.mood = NpcMood::Calm;
                setTarget(npc, -6.10f, 0.0f, 2.25f);
            }
            break;

        case NpcRoutinePhase::WalkNorthBypass:
            npc.activity = NpcActivity::Walking;
            npc.mood = NpcMood::Calm;
            if (moveTowardTarget(npc, deltaSeconds)) {
                npc.routine = NpcRoutinePhase::WalkHome;
                setTarget(npc, npc.homeX, npc.homeY, npc.homeZ);
            }
            break;

        case NpcRoutinePhase::WalkHome:
            npc.activity = NpcActivity::Walking;
            npc.mood = NpcMood::Calm;
            if (moveTowardTarget(npc, deltaSeconds)) {
                npc.routine = NpcRoutinePhase::IdleHome;
                npc.activity = NpcActivity::Idle;
                npc.activitySeconds = 0.0f;
                npc.velocityX = 0.0f;
                npc.velocityZ = 0.0f;
            }
            break;

        case NpcRoutinePhase::IdleHome:
            npc.activity = NpcActivity::Idle;
            npc.mood = NpcMood::Calm;
            npc.velocityX = 0.0f;
            npc.velocityZ = 0.0f;
            npc.activitySeconds += deltaSeconds;
            if (npc.activitySeconds >= kHomeIdleSeconds) {
                npc.activitySeconds = 0.0f;
                ++npc.routineCycles;
                npc.routine = NpcRoutinePhase::WalkToCouch;
                npc.activity = NpcActivity::Walking;
                setTarget(npc, 5.72f, 0.0f, 3.05f);
            }
            break;
    }
}

} // namespace hakui
