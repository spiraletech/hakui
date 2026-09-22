#include "body/BodyConstraintSolver.hpp"

#include <algorithm>
#include <cmath>

namespace hakui::body {
namespace {

constexpr float kPi = 3.14159265358979323846f;
constexpr float kEpsilon = 0.00001f;

BodyPosePoint add(BodyPosePoint a, BodyPosePoint b) noexcept
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}

BodyPosePoint subtract(BodyPosePoint a, BodyPosePoint b) noexcept
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

BodyPosePoint multiply(BodyPosePoint value, float scalar) noexcept
{
    return {value.x * scalar, value.y * scalar, value.z * scalar};
}

float dot(BodyPosePoint a, BodyPosePoint b) noexcept
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float lengthSquared(BodyPosePoint value) noexcept
{
    return dot(value, value);
}

float length(BodyPosePoint value) noexcept
{
    return std::sqrt(lengthSquared(value));
}

float distance(BodyPosePoint a, BodyPosePoint b) noexcept
{
    return length(subtract(a, b));
}

float horizontalDistance(BodyPosePoint a, BodyPosePoint b) noexcept
{
    const float x = a.x - b.x;
    const float z = a.z - b.z;
    return std::sqrt(x * x + z * z);
}

BodyPosePoint normalizeOr(
    BodyPosePoint value,
    BodyPosePoint fallback
) noexcept
{
    const float magnitude = length(value);
    if (magnitude <= kEpsilon) {
        return fallback;
    }
    return multiply(value, 1.0f / magnitude);
}

BodyPosePoint rotateYaw(BodyPosePoint point, float yaw) noexcept
{
    const float cosine = std::cos(yaw);
    const float sine = std::sin(yaw);
    return {
        point.x * cosine + point.z * sine,
        point.y,
        -point.x * sine + point.z * cosine
    };
}

BodyPosePoint inverseRotateYaw(BodyPosePoint point, float yaw) noexcept
{
    return rotateYaw(point, -yaw);
}

BodyPosePoint rootWorld(
    const BodyConstraintInput& input,
    float rootHeight
) noexcept
{
    return {
        input.rootX,
        input.rootY + rootHeight,
        input.rootZ
    };
}

BodyPosePoint localToWorld(
    BodyPosePoint local,
    const BodyConstraintInput& input,
    float rootHeight
) noexcept
{
    return add(rootWorld(input, rootHeight), rotateYaw(local, input.rootYaw));
}

BodyPosePoint worldToLocal(
    BodyPosePoint world,
    const BodyConstraintInput& input,
    float rootHeight
) noexcept
{
    return inverseRotateYaw(
        subtract(world, rootWorld(input, rootHeight)),
        input.rootYaw
    );
}

float wrapRadians(float radians) noexcept
{
    while (radians > kPi) radians -= 2.0f * kPi;
    while (radians < -kPi) radians += 2.0f * kPi;
    return radians;
}

BodyPosePoint groundTarget(
    const FootGroundSample& sample,
    const BodyRigMetrics& rig
) noexcept
{
    const BodyPosePoint normal = normalizeOr(
        sample.normal,
        {0.0f, 1.0f, 0.0f}
    );
    return add(sample.point, multiply(normal, rig.footTargetAboveSurface));
}

void resetFoot(FootConstraintState& foot) noexcept
{
    foot.phase = FootPlantPhase::Swing;
    foot.plantAge = 0.0f;
    foot.releaseBlend = 0.0f;
    foot.contactError = 0.0f;
    foot.reachClamped = false;
}

void beginRelease(FootConstraintState& foot) noexcept
{
    if (foot.phase == FootPlantPhase::Planted) {
        foot.phase = FootPlantPhase::Releasing;
        foot.releaseBlend = 1.0f;
        foot.plantAge = 0.0f;
    }
}

BodyPosePoint chooseFootTarget(
    BodyPosePoint nominalWorld,
    const FootGroundSample& sample,
    const BodyConstraintInput& input,
    FootConstraintState& foot
) noexcept
{
    const BodyRigMetrics& rig = input.rig;
    const float dt = std::max(0.0f, input.deltaTime);

    if (!input.grounded) {
        resetFoot(foot);
        return nominalWorld;
    }

    const bool hasGround = sample.valid;
    const BodyPosePoint candidate = hasGround
        ? groundTarget(sample, rig)
        : nominalWorld;

    if (foot.phase == FootPlantPhase::Planted) {
        foot.plantAge += dt;

        const float yawError = std::fabs(wrapRadians(
            input.rootYaw - foot.rootYawAtPlant
        ));
        const float lift = hasGround
            ? nominalWorld.y - candidate.y
            : rig.plantReleaseHeight + 1.0f;
        const float drift = horizontalDistance(
            nominalWorld,
            foot.worldTarget
        );

        if (!hasGround ||
            lift > rig.plantReleaseHeight ||
            drift > rig.plantReleaseDistance ||
            yawError > rig.plantReleaseYaw) {
            beginRelease(foot);
        } else {
            return foot.worldTarget;
        }
    }

    if (foot.phase == FootPlantPhase::Releasing) {
        foot.releaseBlend = std::max(
            0.0f,
            foot.releaseBlend - dt * rig.plantReleaseSpeed
        );
        const BodyPosePoint target = add(
            multiply(nominalWorld, 1.0f - foot.releaseBlend),
            multiply(foot.worldTarget, foot.releaseBlend)
        );
        if (foot.releaseBlend <= 0.0f) {
            foot.phase = FootPlantPhase::Swing;
        }
        return target;
    }

    if (hasGround) {
        const float verticalError = std::fabs(
            nominalWorld.y - candidate.y
        );
        const float horizontalError = horizontalDistance(
            nominalWorld,
            candidate
        );
        const bool nearSurface =
            verticalError <= rig.plantAcquireHeight &&
            horizontalError <= rig.plantAcquireDistance;
        const bool stableIdle =
            input.movementBlend <= 0.08f &&
            verticalError <= rig.maxPelvisDrop &&
            horizontalError <= rig.plantAcquireDistance;

        if (nearSurface || stableIdle) {
            foot.phase = FootPlantPhase::Planted;
            foot.worldTarget = candidate;
            foot.groundNormal = normalizeOr(
                sample.normal,
                {0.0f, 1.0f, 0.0f}
            );
            foot.rootYawAtPlant = input.rootYaw;
            foot.plantAge = 0.0f;
            foot.releaseBlend = 0.0f;
            return foot.worldTarget;
        }
    }

    return nominalWorld;
}

float requiredPelvisDrop(
    BodyPosePoint hipWorld,
    BodyPosePoint footWorld,
    const BodyRigMetrics& rig
) noexcept
{
    const float maximumReach = std::max(
        0.01f,
        rig.upperLegLength +
            rig.lowerLegLength -
            rig.legReachMargin
    );
    const float horizontal = horizontalDistance(hipWorld, footWorld);
    if (horizontal >= maximumReach) {
        return 0.0f;
    }

    const float vertical = hipWorld.y - footWorld.y;
    if (vertical <= 0.0f) {
        return 0.0f;
    }

    const float allowedVertical = std::sqrt(std::max(
        0.0f,
        maximumReach * maximumReach - horizontal * horizontal
    ));
    return std::max(0.0f, vertical - allowedVertical);
}

float smoothToward(
    float current,
    float target,
    float rate,
    float dt
) noexcept
{
    if (dt <= 0.0f) {
        return target;
    }
    const float alpha = 1.0f - std::exp(-std::max(0.0f, rate) * dt);
    return current + (target - current) * alpha;
}

BodyLimbPose solveTwoBoneLeg(
    const BodyLimbPose& nominal,
    BodyPosePoint requestedFoot,
    const BodyRigMetrics& rig,
    bool& reachClamped
) noexcept
{
    BodyLimbPose solved = nominal;
    const BodyPosePoint hip = nominal.proximal;

    const float upper = std::max(0.01f, rig.upperLegLength);
    const float lower = std::max(0.01f, rig.lowerLegLength);
    const float maximumReach = std::max(
        0.01f,
        upper + lower - rig.legReachMargin
    );
    const float minimumReach = std::max(
        0.01f,
        std::fabs(upper - lower) + rig.legReachMargin
    );

    BodyPosePoint hipToFoot = subtract(requestedFoot, hip);
    float requestedDistance = length(hipToFoot);
    BodyPosePoint direction = normalizeOr(
        hipToFoot,
        {0.0f, -1.0f, 0.0f}
    );

    const float solvedDistance = std::clamp(
        requestedDistance,
        minimumReach,
        maximumReach
    );
    reachClamped = std::fabs(solvedDistance - requestedDistance) > 0.0001f;

    const BodyPosePoint foot = add(
        hip,
        multiply(direction, solvedDistance)
    );

    const float along = (
        upper * upper -
        lower * lower +
        solvedDistance * solvedDistance
    ) / (2.0f * solvedDistance);
    const float kneeRadius = std::sqrt(std::max(
        0.0f,
        upper * upper - along * along
    ));

    BodyPosePoint bendPreference = rotateYaw(
        {0.0f, 0.0f, 1.0f},
        nominal.distalYaw
    );
    bendPreference = subtract(
        bendPreference,
        multiply(direction, dot(bendPreference, direction))
    );
    bendPreference = normalizeOr(
        bendPreference,
        {1.0f, 0.0f, 0.0f}
    );

    solved.proximal = hip;
    solved.middle = add(
        add(hip, multiply(direction, along)),
        multiply(bendPreference, kneeRadius)
    );
    solved.distal = foot;
    solved.distalYaw = nominal.distalYaw;
    return solved;
}

void updateFootTelemetry(
    FootConstraintState& state,
    const BodyLimbPose& limb,
    BodyPosePoint requestedWorldTarget,
    const BodyConstraintInput& input,
    float rootHeight,
    bool reachClamped
) noexcept
{
    const BodyPosePoint solvedWorld = localToWorld(
        limb.distal,
        input,
        rootHeight
    );
    state.contactError = distance(
        solvedWorld,
        requestedWorldTarget
    );
    state.reachClamped = reachClamped;
}

} // namespace

BodyPoseState BodyConstraintSolver::solve(
    const BodyPoseState& nominalPose,
    const BodyConstraintInput& input,
    BodyConstraintState& state
) const noexcept
{
    if (!nominalPose.valid) {
        return nominalPose;
    }

    if (!input.enabled) {
        state = {};
        state.initialized = true;
        state.wasGrounded = input.grounded;
        return nominalPose;
    }

    const bool landed =
        state.initialized &&
        !state.wasGrounded &&
        input.grounded;

    if (!state.initialized) {
        state.initialized = true;
        state.wasGrounded = input.grounded;
    }

    if (landed) {
        state.landingCompression = input.rig.landingCompression;
    }
    if (!input.grounded) {
        state.landingCompression = 0.0f;
    } else {
        state.landingCompression = std::max(
            0.0f,
            state.landingCompression -
                std::max(0.0f, input.deltaTime) *
                input.rig.landingRecoverySpeed
        );
    }

    const BodyPosePoint nominalLeftWorld = localToWorld(
        nominalPose.leftLeg.distal,
        input,
        nominalPose.rootHeightAbovePlayerBase
    );
    const BodyPosePoint nominalRightWorld = localToWorld(
        nominalPose.rightLeg.distal,
        input,
        nominalPose.rootHeightAbovePlayerBase
    );

    BodyPosePoint leftTargetWorld = chooseFootTarget(
        nominalLeftWorld,
        input.leftGround,
        input,
        state.leftFoot
    );
    BodyPosePoint rightTargetWorld = chooseFootTarget(
        nominalRightWorld,
        input.rightGround,
        input,
        state.rightFoot
    );

    const BodyPosePoint nominalLeftHipWorld = localToWorld(
        nominalPose.leftLeg.proximal,
        input,
        nominalPose.rootHeightAbovePlayerBase
    );
    const BodyPosePoint nominalRightHipWorld = localToWorld(
        nominalPose.rightLeg.proximal,
        input,
        nominalPose.rootHeightAbovePlayerBase
    );

    float desiredDrop = std::max(
        requiredPelvisDrop(
            nominalLeftHipWorld,
            leftTargetWorld,
            input.rig
        ),
        requiredPelvisDrop(
            nominalRightHipWorld,
            rightTargetWorld,
            input.rig
        )
    );
    desiredDrop = std::clamp(
        desiredDrop,
        0.0f,
        input.rig.maxPelvisDrop
    );

    const float pelvisRate = desiredDrop > state.pelvisDrop
        ? input.rig.pelvisAcquireRate
        : input.rig.pelvisRecoveryRate;
    state.pelvisDrop = smoothToward(
        state.pelvisDrop,
        desiredDrop,
        pelvisRate,
        std::max(0.0f, input.deltaTime)
    );

    BodyPoseState corrected = nominalPose;
    const float totalDrop =
        state.pelvisDrop + state.landingCompression;
    corrected.rootHeightAbovePlayerBase = std::max(
        0.05f,
        nominalPose.rootHeightAbovePlayerBase - totalDrop
    );

    const BodyPosePoint leftTargetLocal = worldToLocal(
        leftTargetWorld,
        input,
        corrected.rootHeightAbovePlayerBase
    );
    const BodyPosePoint rightTargetLocal = worldToLocal(
        rightTargetWorld,
        input,
        corrected.rootHeightAbovePlayerBase
    );

    bool leftClamped = false;
    bool rightClamped = false;
    corrected.leftLeg = solveTwoBoneLeg(
        nominalPose.leftLeg,
        leftTargetLocal,
        input.rig,
        leftClamped
    );
    corrected.rightLeg = solveTwoBoneLeg(
        nominalPose.rightLeg,
        rightTargetLocal,
        input.rig,
        rightClamped
    );

    updateFootTelemetry(
        state.leftFoot,
        corrected.leftLeg,
        leftTargetWorld,
        input,
        corrected.rootHeightAbovePlayerBase,
        leftClamped
    );
    updateFootTelemetry(
        state.rightFoot,
        corrected.rightLeg,
        rightTargetWorld,
        input,
        corrected.rootHeightAbovePlayerBase,
        rightClamped
    );

    corrected.grounded = input.grounded;
    corrected.airborne = !input.grounded;
    state.wasGrounded = input.grounded;
    return corrected;
}

} // namespace hakui::body
