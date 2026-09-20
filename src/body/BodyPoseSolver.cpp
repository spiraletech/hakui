#include "body/BodyPoseSolver.hpp"

#include <algorithm>
#include <cmath>

#include "avatar/AvatarGroundContact.hpp"

namespace hakui::body {
namespace {

constexpr float kPi = 3.14159265358979323846f;

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

BodyLimbPose solveContactLeg(
    float side,
    BodyPosePoint target,
    float kneeFlex,
    float hipYaw,
    float footYaw,
    float rootHeight,
    float preloadDrop,
    float landingDrop
) noexcept
{
    target.y -= rootHeight;
    const float flex = std::clamp(kneeFlex, 0.0f, 1.35f);
    const BodyPosePoint hip = rotateYaw(
        {
            side * 0.23f,
            1.17f - flex * 0.045f - preloadDrop - landingDrop,
            0.0f
        },
        hipYaw
    );

    const BodyPosePoint bendForward =
        rotateYaw({0.0f, 0.0f, 1.0f}, footYaw);
    const BodyPosePoint outward =
        rotateYaw({side, 0.0f, 0.0f}, hipYaw);
    const float bend = 0.08f + flex * 0.15f;
    const BodyPosePoint knee{
        (hip.x + target.x) * 0.5f +
            bendForward.x * bend + outward.x * 0.045f,
        (hip.y + target.y) * 0.5f + 0.12f - flex * 0.055f,
        (hip.z + target.z) * 0.5f +
            bendForward.z * bend + outward.z * 0.045f
    };
    return {hip, knee, target, footYaw};
}

BodyLimbPose solveContactArm(
    float side,
    BodyPosePoint target,
    float shoulderYaw,
    float elbowFlex,
    const avatar::BodyProfile& body,
    float rootHeight
) noexcept
{
    target.y -= rootHeight;
    const BodyPosePoint shoulder = rotateYaw(
        {
            side * body.shoulderHalfWidth,
            body.shoulderHeight,
            0.0f
        },
        shoulderYaw
    );
    const BodyPosePoint elbow{
        (shoulder.x + target.x) * 0.5f + side * body.elbowOut,
        (shoulder.y + target.y) * 0.5f - 0.04f - elbowFlex * 0.06f,
        (shoulder.z + target.z) * 0.5f - 0.08f - elbowFlex * 0.08f
    };
    return {shoulder, elbow, target, 0.0f};
}

EmbodimentProfileId embodimentFor(const PlayerState& player) noexcept
{
    if (player.activity != PlayerActivity::Roaming) {
        return EmbodimentProfileId::Seated;
    }
    switch (player.locomotion) {
    case LocomotionMode::Skateboard: return EmbodimentProfileId::Skateboard;
    case LocomotionMode::BMX: return EmbodimentProfileId::Bmx;
    case LocomotionMode::OnFoot:
    case LocomotionMode::Car:
        return EmbodimentProfileId::OnFoot;
    }
    return EmbodimentProfileId::OnFoot;
}

} // namespace

BodyPoseState BodyPoseSolver::solve(
    const PlayerState& player,
    const BodyPoseSolveInput& input
) const noexcept
{
    BodyPoseState pose;
    const avatar::BodyProfile& body = avatar::bodyProfile(input.bodyProfile);
    const AvatarGroundContactProfile& contact =
        avatarGroundContactProfile(embodimentFor(player));

    pose.valid = true;
    pose.grounded = player.grounded;
    pose.airborne = !player.grounded;

    const float groundedBlend = player.grounded ? 1.0f : 0.20f;
    const float gait = std::sin(player.gaitPhase);
    const float counterGait = std::sin(player.gaitPhase + kPi);
    const float bodyBob =
        0.045f * std::abs(gait) * player.movementBlend * groundedBlend;
    const float idleBreath = 0.012f * std::sin(player.idlePhase);

    const bool authored = input.preset != BodyPosePreset::Locomotion;
    const RideBodyMechanicsState& mechanics = input.mechanics;
    pose.rootHeightAbovePlayerBase =
        contact.visualRootAbovePlayerBase + (authored ? 0.0f : bodyBob);

    pose.pelvisYaw = authored ? mechanics.pelvisYawRelativeToBoard : 0.0f;
    pose.torsoYaw = authored ? mechanics.torsoYawRelativeToBoard : 0.0f;
    pose.headYaw = authored ? mechanics.headYawRelativeToBoard : 0.0f;

    const float preloadDrop = authored
        ? mechanics.preloadPoseWeight * 0.18f
        : 0.0f;
    const float landingDrop = authored
        ? mechanics.landingCompression * 0.15f
        : 0.0f;
    const float rideCompression = authored
        ? mechanics.preloadPoseWeight * 0.27f +
            mechanics.landingCompression * 0.28f
        : 0.0f;

    pose.torsoPitch =
        (player.grounded
            ? 0.0f
            : std::clamp(-player.velocityY * 0.035f, -0.18f, 0.30f)) +
        (authored ? mechanics.torsoLean : 0.0f);
    pose.torsoRoll = authored
        ? 0.0f
        : 0.035f * gait * player.movementBlend;

    pose.pelvisCenter = {
        0.0f,
        1.20f - (authored ? mechanics.preloadPoseWeight * 0.10f : 0.0f) -
            (authored ? mechanics.landingCompression * 0.18f : 0.0f),
        0.0f
    };
    pose.waistCenter = {
        0.0f,
        1.32f - (authored ? mechanics.preloadPoseWeight * 0.16f : 0.0f) -
            (authored ? mechanics.landingCompression * 0.13f : 0.0f),
        0.0f
    };
    pose.torsoCenter = {
        0.0f,
        1.72f + idleBreath - rideCompression,
        0.0f
    };
    pose.clavicleCenter =
        rotateYaw({0.0f, 2.12f - rideCompression, 0.0f}, pose.torsoYaw);
    pose.neckCenter = {
        0.0f,
        2.28f + idleBreath - rideCompression,
        0.0f
    };
    pose.headCenter = {
        0.0f,
        2.60f + idleBreath - rideCompression,
        0.0f
    };

    const float footSpread =
        input.bodyProfile == avatar::BodyProfileId::Female ? 0.255f : 0.23f;
    BodyPosePoint leftFoot{};
    BodyPosePoint rightFoot{};
    float leftKneeFlex = 0.18f;
    float rightKneeFlex = 0.18f;
    float legYaw = pose.pelvisYaw;

    if (authored) {
        leftFoot = rotateYaw({-footSpread, 0.14f, 0.04f}, legYaw);
        rightFoot = rotateYaw({footSpread, 0.14f, 0.04f}, legYaw);
        leftKneeFlex = mechanics.leftKneeFlex;
        rightKneeFlex = mechanics.rightKneeFlex;
        if (input.preset == BodyPosePreset::OlliePop) {
            leftFoot.y += mechanics.frontFootLift;
            rightFoot.y += mechanics.rearLegDrive * 0.10f;
            rightFoot.z -= mechanics.rearLegDrive * 0.12f;
        }
    } else {
        const float stride = 0.42f * player.movementBlend * groundedBlend;
        const float lift = 0.13f * player.movementBlend * groundedBlend;
        leftFoot = {
            -footSpread,
            0.14f + std::max(0.0f, -gait) * lift,
            gait * stride
        };
        rightFoot = {
            footSpread,
            0.14f + std::max(0.0f, -counterGait) * lift,
            counterGait * stride
        };
        leftKneeFlex = 0.16f +
            std::max(0.0f, -gait) * 0.38f * player.movementBlend;
        rightKneeFlex = 0.16f +
            std::max(0.0f, -counterGait) * 0.38f * player.movementBlend;
        if (!player.grounded) {
            leftKneeFlex = std::max(leftKneeFlex, 0.42f);
            rightKneeFlex = std::max(rightKneeFlex, 0.42f);
            leftFoot.y += 0.08f;
            rightFoot.y += 0.08f;
        }
    }

    pose.leftLeg = solveContactLeg(
        -1.0f, leftFoot, leftKneeFlex, legYaw, legYaw,
        pose.rootHeightAbovePlayerBase, preloadDrop, landingDrop
    );
    pose.rightLeg = solveContactLeg(
        1.0f, rightFoot, rightKneeFlex, legYaw, legYaw,
        pose.rootHeightAbovePlayerBase, preloadDrop, landingDrop
    );

    const float neutralHandX =
        input.bodyProfile == avatar::BodyProfileId::Female ? 0.56f : 0.62f;
    BodyPosePoint leftHand{-neutralHandX, 1.56f, 0.02f};
    BodyPosePoint rightHand{neutralHandX, 1.56f, 0.02f};
    float leftElbowFlex = authored ? mechanics.leftElbowFlex : 0.10f;
    float rightElbowFlex = authored ? mechanics.rightElbowFlex : 0.10f;

    if (authored) {
        switch (input.preset) {
        case BodyPosePreset::TPose:
            leftHand = {
                input.bodyProfile == avatar::BodyProfileId::Female ? -1.18f : -1.28f,
                input.bodyProfile == avatar::BodyProfileId::Female ? 2.02f : 2.04f,
                0.0f
            };
            rightHand = {
                -leftHand.x,
                leftHand.y,
                0.0f
            };
            break;
        case BodyPosePreset::APose:
            leftHand = {
                input.bodyProfile == avatar::BodyProfileId::Female ? -0.96f : -1.05f,
                input.bodyProfile == avatar::BodyProfileId::Female ? 1.70f : 1.72f,
                0.0f
            };
            rightHand = {-leftHand.x, leftHand.y, 0.0f};
            break;
        case BodyPosePreset::Crouch:
            leftHand = {
                input.bodyProfile == avatar::BodyProfileId::Female ? -0.66f : -0.72f,
                1.40f,
                -0.08f
            };
            rightHand = {-leftHand.x, 1.40f, -0.08f};
            break;
        case BodyPosePreset::OlliePop:
            leftHand = {
                input.bodyProfile == avatar::BodyProfileId::Female ? -0.84f : -0.92f,
                1.58f,
                -0.14f
            };
            rightHand = {
                input.bodyProfile == avatar::BodyProfileId::Female ? 0.84f : 0.92f,
                1.68f,
                0.18f
            };
            break;
        case BodyPosePreset::Locomotion:
        case BodyPosePreset::Neutral:
            break;
        }
    } else {
        const float armSwing = 0.34f * player.movementBlend * groundedBlend;
        leftHand.z += counterGait * armSwing;
        rightHand.z += gait * armSwing;
        leftHand.y -= std::abs(counterGait) * 0.05f * player.movementBlend;
        rightHand.y -= std::abs(gait) * 0.05f * player.movementBlend;
        if (!player.grounded) {
            leftElbowFlex = 0.24f;
            rightElbowFlex = 0.24f;
            leftHand.y += 0.08f;
            rightHand.y += 0.08f;
        }
    }

    leftHand = rotateYaw(leftHand, pose.torsoYaw);
    rightHand = rotateYaw(rightHand, pose.torsoYaw);
    pose.leftArm = solveContactArm(
        -1.0f, leftHand, pose.torsoYaw, leftElbowFlex,
        body, pose.rootHeightAbovePlayerBase
    );
    pose.rightArm = solveContactArm(
        1.0f, rightHand, pose.torsoYaw, rightElbowFlex,
        body, pose.rootHeightAbovePlayerBase
    );

    return pose;
}

} // namespace hakui::body
