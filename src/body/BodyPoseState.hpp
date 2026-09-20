#pragma once

#include <cstdint>

namespace hakui::body {

struct BodyPosePoint {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

struct BodyLimbPose {
    BodyPosePoint proximal{};
    BodyPosePoint middle{};
    BodyPosePoint distal{};
    float distalYaw = 0.0f;
};

struct BodyPoseState {
    bool valid = false;
    bool grounded = true;
    bool airborne = false;

    // Root offset is measured above PlayerState::y. Rendering applies the
    // player world transform; HAKUI owns the body-space solution below.
    float rootHeightAbovePlayerBase = 0.0f;

    BodyPosePoint pelvisCenter{};
    BodyPosePoint waistCenter{};
    BodyPosePoint torsoCenter{};
    BodyPosePoint clavicleCenter{};
    BodyPosePoint neckCenter{};
    BodyPosePoint headCenter{};

    float pelvisYaw = 0.0f;
    float torsoYaw = 0.0f;
    float headYaw = 0.0f;
    float torsoPitch = 0.0f;
    float torsoRoll = 0.0f;

    BodyLimbPose leftLeg{};
    BodyLimbPose rightLeg{};
    BodyLimbPose leftArm{};
    BodyLimbPose rightArm{};
};

enum class BodyPosePreset : std::uint8_t {
    Locomotion,
    Neutral,
    TPose,
    APose,
    Crouch,
    OlliePop,
};

} // namespace hakui::body
