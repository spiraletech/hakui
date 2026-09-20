#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "character/CharacterEmbodiment.hpp"
#include "character/CharacterPerformanceProfile.hpp"

namespace hakui::character {

enum class PoseJoint : std::uint8_t {
    Root,
    Pelvis,
    SpineLower,
    SpineUpper,
    Neck,
    Skull,
    Jaw,
    LeftShoulder,
    RightShoulder,
    LeftUpperArm,
    RightUpperArm,
    LeftForearm,
    RightForearm,
    Hood,
    Cloak,
};

struct PoseChannel {
    PoseJoint joint = PoseJoint::Root;
    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;
    float weight = 0.0f;
};

// L22 is the first concrete bridge from authored character performance into
// rig-space pose directives. It still does not own a transform, animation
// clock, root motion, combat state, renderer resource, or persistence record.
// Callers provide a normalized presentation phase and remain authoritative for
// when/where the resulting pose is played.
struct CharacterPoseExecution {
    CharacterId characterId = CharacterId::None;
    CharacterRigArchetype rig = CharacterRigArchetype::Humanoid;
    CharacterExpression expression = CharacterExpression::Neutral;
    bool active = false;
    bool skeletalOnly = false;
    bool usesHumanFacialMuscles = false;
    bool usesRasterEyeChannel = false;

    float rasterEyeWeight = 0.0f;
    float jawOpen = 0.0f;
    float snapWeight = 0.0f;
    float staccatoWeight = 0.0f;

    std::array<PoseChannel, 16> channels{};
    std::uint8_t channelCount = 0;
};

[[nodiscard]] constexpr float poseClamp01(float value) noexcept
{
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

[[nodiscard]] constexpr float normalizedPosePhase(float phase01) noexcept
{
    if (phase01 < 0.0f) return 0.0f;
    if (phase01 > 1.0f) return 1.0f;
    return phase01;
}

// Dependency-free triangle sample: 0 -> 1 -> 0 over one normalized cycle.
[[nodiscard]] constexpr float poseTriangle(float phase01) noexcept
{
    const float phase = normalizedPosePhase(phase01);
    return phase <= 0.5f ? phase * 2.0f : (1.0f - phase) * 2.0f;
}

// Reaper intentionally moves in discontinuous held frames rather than smooth
// human interpolation. The result is deterministic and driven by caller phase.
[[nodiscard]] constexpr float reaperStaccatoSample(float phase01) noexcept
{
    const float phase = normalizedPosePhase(phase01);
    if (phase < 0.125f) return -1.0f;
    if (phase < 0.250f) return 0.35f;
    if (phase < 0.375f) return -0.55f;
    if (phase < 0.500f) return 0.85f;
    if (phase < 0.625f) return -0.20f;
    if (phase < 0.750f) return 1.0f;
    if (phase < 0.875f) return -0.75f;
    return 0.15f;
}

constexpr void appendPoseChannel(
    CharacterPoseExecution& execution,
    PoseJoint joint,
    float pitch,
    float yaw,
    float roll,
    float weight = 1.0f
) noexcept
{
    if (execution.channelCount >= execution.channels.size()) return;
    execution.channels[execution.channelCount++] = PoseChannel{
        joint,
        pitch,
        yaw,
        roll,
        poseClamp01(weight),
    };
}

[[nodiscard]] constexpr const PoseChannel* findPoseChannel(
    const CharacterPoseExecution& execution,
    PoseJoint joint
) noexcept
{
    for (std::size_t index = 0; index < execution.channelCount; ++index) {
        if (execution.channels[index].joint == joint) {
            return &execution.channels[index];
        }
    }
    return nullptr;
}

[[nodiscard]] constexpr CharacterPoseExecution executeCharacterPerformance(
    const CharacterPerformanceFrame& frame,
    float phase01
) noexcept
{
    const CharacterPerformanceProfile* performance =
        canonicalPerformanceProfile(frame.characterId);
    const CharacterEmbodimentProfile* embodiment =
        canonicalEmbodiment(frame.characterId);
    if (!performance || !embodiment) return {};

    CharacterPoseExecution execution{};
    execution.characterId = frame.characterId;
    execution.rig = embodiment->rig;
    execution.expression = frame.expression;
    execution.active = true;
    execution.skeletalOnly = embodiment->rig == CharacterRigArchetype::ReaperSkeleton;
    execution.usesHumanFacialMuscles = performance->usesHumanFacialMuscles;
    execution.usesRasterEyeChannel = performance->usesRasterEyeChannel;
    execution.rasterEyeWeight = performance->usesRasterEyeChannel
        ? poseClamp01(frame.cranialIntensity)
        : 0.0f;
    execution.jawOpen = performance->usesJawChannel
        ? poseClamp01(frame.jawOpen)
        : 0.0f;
    execution.snapWeight = poseClamp01(frame.snapWeight);
    execution.staccatoWeight = poseClamp01(frame.staccatoWeight);

    const float breathe = (poseTriangle(phase01) - 0.5f) *
        frame.idleAmplitude;

    switch (frame.characterId) {
        case CharacterId::Agnathos: {
            // Guardian Stillness keeps movement concentrated in the upper spine,
            // neck, hands and Raster eyes rather than exaggerated full-body sway.
            appendPoseChannel(
                execution, PoseJoint::SpineLower,
                frame.torsoPitch * 0.38f + breathe * 0.012f,
                0.0f,
                frame.torsoRoll * 0.34f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::SpineUpper,
                frame.torsoPitch * 0.72f + breathe * 0.018f,
                0.0f,
                frame.torsoRoll * 0.70f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::Neck,
                frame.headPitch * 0.54f,
                0.0f,
                frame.headRoll * 0.48f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::Skull,
                frame.headPitch * 0.46f,
                0.0f,
                frame.headRoll * 0.52f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::LeftUpperArm,
                frame.leftArmBias * frame.gestureWeight,
                0.0f,
                -0.025f * frame.gestureWeight,
                0.92f
            );
            appendPoseChannel(
                execution, PoseJoint::RightUpperArm,
                frame.rightArmBias * frame.gestureWeight,
                0.0f,
                0.025f * frame.gestureWeight,
                0.92f
            );
            appendPoseChannel(
                execution, PoseJoint::Jaw,
                execution.jawOpen * 0.12f,
                0.0f,
                0.0f,
                execution.jawOpen
            );
            break;
        }

        case CharacterId::Saelis: {
            // Synthetic Precision damps free sway and lets pose snap provide the
            // authored machine-like punctuation between otherwise smooth poses.
            appendPoseChannel(
                execution, PoseJoint::SpineLower,
                frame.torsoPitch * 0.30f,
                0.0f,
                frame.torsoRoll * 0.26f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::SpineUpper,
                frame.torsoPitch * 0.62f + breathe * 0.008f,
                0.0f,
                frame.torsoRoll * 0.58f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::Neck,
                frame.headPitch * 0.44f,
                0.0f,
                frame.headRoll * 0.42f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::Skull,
                frame.headPitch * 0.56f,
                execution.snapWeight * 0.025f,
                frame.headRoll * 0.58f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::LeftUpperArm,
                frame.leftArmBias * frame.gestureWeight,
                0.0f,
                -0.015f * frame.gestureWeight,
                0.88f
            );
            appendPoseChannel(
                execution, PoseJoint::RightUpperArm,
                frame.rightArmBias * frame.gestureWeight,
                0.0f,
                0.015f * frame.gestureWeight,
                0.88f
            );
            appendPoseChannel(
                execution, PoseJoint::Jaw,
                execution.jawOpen * 0.10f,
                0.0f,
                0.0f,
                execution.jawOpen
            );
            break;
        }

        case CharacterId::Reaper: {
            // No skin or human facial-muscle channel exists here. The visible
            // performance is skull + jaw + cervical/spine + bony shoulders +
            // hood/cloak displacement, with held-frame discontinuity.
            const float cut = reaperStaccatoSample(phase01) *
                execution.staccatoWeight;
            appendPoseChannel(
                execution, PoseJoint::SpineLower,
                frame.torsoPitch * 0.42f + cut * 0.020f,
                cut * 0.018f,
                frame.torsoRoll * 0.38f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::SpineUpper,
                frame.torsoPitch * 0.76f + cut * 0.045f,
                cut * 0.035f,
                frame.torsoRoll * 0.74f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::Neck,
                frame.headPitch * 0.52f + cut * 0.032f,
                cut * 0.028f,
                frame.headRoll * 0.46f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::Skull,
                frame.headPitch * 0.68f + cut * 0.070f,
                cut * 0.055f,
                frame.headRoll * 0.72f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::Jaw,
                execution.jawOpen * 0.42f,
                0.0f,
                cut * 0.015f,
                execution.jawOpen
            );
            appendPoseChannel(
                execution, PoseJoint::LeftShoulder,
                frame.leftArmBias * 0.36f,
                cut * 0.020f,
                -0.06f - execution.snapWeight * 0.05f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::RightShoulder,
                frame.rightArmBias * 0.36f,
                -cut * 0.020f,
                0.06f + execution.snapWeight * 0.05f,
                1.0f
            );
            appendPoseChannel(
                execution, PoseJoint::LeftUpperArm,
                frame.leftArmBias,
                cut * 0.035f,
                -0.08f,
                0.96f
            );
            appendPoseChannel(
                execution, PoseJoint::RightUpperArm,
                frame.rightArmBias,
                -cut * 0.035f,
                0.08f,
                0.96f
            );
            appendPoseChannel(
                execution, PoseJoint::Hood,
                cut * 0.045f,
                -cut * 0.050f,
                cut * 0.035f,
                0.90f
            );
            appendPoseChannel(
                execution, PoseJoint::Cloak,
                -frame.torsoPitch * 0.18f,
                cut * 0.040f,
                -frame.torsoRoll * 0.22f + cut * 0.055f,
                0.84f
            );
            break;
        }

        case CharacterId::None:
            return {};
    }

    return execution;
}

[[nodiscard]] constexpr CharacterPoseExecution executeCharacterPerformance(
    CharacterId characterId,
    CharacterPerformanceEvent event,
    float phase01
) noexcept
{
    return executeCharacterPerformance(
        composeCharacterPerformance(characterId, event),
        phase01
    );
}

[[nodiscard]] constexpr CharacterPoseExecution executeCharacterPerformance(
    CharacterId characterId,
    MangaRenderEvent event,
    float phase01
) noexcept
{
    return executeCharacterPerformance(
        composeCharacterPerformance(characterId, event),
        phase01
    );
}

} // namespace hakui::character
