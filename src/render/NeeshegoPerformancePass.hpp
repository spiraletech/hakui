#pragma once

#include <algorithm>
#include <cmath>

#include <SDL3/SDL.h>

#include "character/CharacterPoseExecutor.hpp"
#include "render/NeeshegoRenderPass.hpp"
#include "render/Math3D.hpp"

namespace hakui::render {

[[nodiscard]] inline float neeshegoPhase01(float phase) noexcept
{
    constexpr float invTau = 0.15915494309189535f;
    float normalized = std::fmod(std::fabs(phase) * invTau, 1.0f);
    if (normalized < 0.0f) normalized += 1.0f;
    return normalized;
}

[[nodiscard]] inline character::CharacterPerformanceEvent neeshegoPlayerPerformanceEvent(
    const HakuiSceneState& scene,
    const PlayerState& player
) noexcept
{
    const character::MangaRenderEvent renderEvent = neeshegoRenderEvent(scene);
    if (renderEvent != character::MangaRenderEvent::Exploration) {
        return character::performanceEventFor(renderEvent);
    }
    if (player.movementBlend > 0.08f) {
        return character::CharacterPerformanceEvent::Locomotion;
    }
    return character::CharacterPerformanceEvent::Idle;
}

[[nodiscard]] inline character::CharacterPerformanceEvent neeshegoNpcPerformanceEvent(
    const NpcState& npc
) noexcept
{
    switch (npc.activity) {
        case NpcActivity::Walking:
            return character::CharacterPerformanceEvent::Locomotion;
        case NpcActivity::ObservingPlayer:
        case NpcActivity::ObservingSpiral:
            return character::CharacterPerformanceEvent::Dialogue;
        case NpcActivity::Idle:
        case NpcActivity::Seated:
            return character::CharacterPerformanceEvent::Idle;
    }
    return character::CharacterPerformanceEvent::Idle;
}

[[nodiscard]] inline const character::PoseChannel* neeshegoPoseChannel(
    const character::CharacterPoseExecution& pose,
    character::PoseJoint joint
) noexcept
{
    return character::findPoseChannel(pose, joint);
}

inline void neeshegoDrawPerformanceRig(
    SDL_GPUCommandBuffer* commands,
    SDL_GPURenderPass* pass,
    SDL_GPUBuffer* cubeVertexBuffer,
    SDL_GPUGraphicsPipeline* opaquePipeline,
    Uint32 cubeVertexCount,
    const math::Mat4& viewProjection,
    const math::Mat4& root,
    const character::CharacterPoseExecution& pose,
    bool fullSkeleton
) noexcept
{
    using namespace hakui::math;
    if (!commands || !pass || !cubeVertexBuffer || !opaquePipeline ||
        cubeVertexCount == 0 || !pose.active) {
        return;
    }

    SDL_GPUBufferBinding binding{};
    binding.buffer = cubeVertexBuffer;
    binding.offset = 0;
    SDL_BindGPUVertexBuffers(pass, 0, &binding, 1);
    SDL_BindGPUGraphicsPipeline(pass, opaquePipeline);

    const auto drawModel = [&](const Mat4& model, Uint32 palette) {
        const Mat4 mvp = multiply(viewProjection, model);
        SDL_PushGPUVertexUniformData(commands, 0, mvp.m, sizeof(mvp.m));
        SDL_DrawGPUPrimitives(
            pass,
            cubeVertexCount,
            1,
            palette * cubeVertexCount,
            0
        );
    };

    const auto poseRotation = [&](character::PoseJoint joint) {
        const character::PoseChannel* channel = neeshegoPoseChannel(pose, joint);
        if (!channel) return identity();
        return multiply(
            rotationY(channel->yaw * channel->weight),
            multiply(
                rotationX(channel->pitch * channel->weight),
                rotationZ(channel->roll * channel->weight)
            )
        );
    };

    const auto localBox = [&](const Vec3& position,
                              const Vec3& dimensions,
                              character::PoseJoint joint,
                              Uint32 palette) {
        const Mat4 model = multiply(
            root,
            multiply(
                translation(position),
                multiply(poseRotation(joint), scale(dimensions))
            )
        );
        drawModel(model, palette);
    };

    if (pose.rig == character::CharacterRigArchetype::Humanoid) {
        // These are thin authored ink-shell accents over the established HAKUI
        // procedural body. They execute real L22 pose channels without taking
        // root-motion ownership away from PlayerRuntime/NpcManager.
        localBox(
            {0.0f, 1.46f, 0.0f},
            {0.12f, 0.44f, 0.12f},
            character::PoseJoint::SpineLower,
            neeshegoMidnightPalette
        );
        localBox(
            {0.0f, 1.90f, 0.0f},
            {0.16f, 0.56f, 0.14f},
            character::PoseJoint::SpineUpper,
            neeshegoMidnightPalette
        );
        localBox(
            {0.0f, 2.30f, 0.0f},
            {0.16f, 0.22f, 0.16f},
            character::PoseJoint::Neck,
            neeshegoShellPalette
        );
        localBox(
            {0.0f, 2.61f, 0.0f},
            {0.58f, 0.56f, 0.54f},
            character::PoseJoint::Skull,
            neeshegoShellPalette
        );
        localBox(
            {-0.58f, 1.86f, 0.0f},
            {0.16f, 0.66f, 0.16f},
            character::PoseJoint::LeftUpperArm,
            neeshegoMidnightPalette
        );
        localBox(
            {0.58f, 1.86f, 0.0f},
            {0.16f, 0.66f, 0.16f},
            character::PoseJoint::RightUpperArm,
            neeshegoMidnightPalette
        );

        if (pose.usesRasterEyeChannel && pose.rasterEyeWeight > 0.02f) {
            const float eyeWidth = 0.14f + pose.rasterEyeWeight * 0.10f;
            localBox(
                {-0.16f, 2.66f, 0.30f},
                {eyeWidth, 0.045f, 0.035f},
                character::PoseJoint::Skull,
                neeshegoMidnightPalette
            );
            localBox(
                {0.16f, 2.66f, 0.30f},
                {eyeWidth, 0.045f, 0.035f},
                character::PoseJoint::Skull,
                neeshegoMidnightPalette
            );
        }

        if (pose.jawOpen > 0.01f) {
            localBox(
                {0.0f, 2.42f - pose.jawOpen * 0.05f, 0.19f},
                {0.30f, 0.08f + pose.jawOpen * 0.08f, 0.16f},
                character::PoseJoint::Jaw,
                neeshegoMidnightPalette
            );
        }
        return;
    }

    if (!fullSkeleton) return;

    // THE REAPER: literal bone-only renderer. There is no torso-skin block,
    // human face shell, hair slot, or flesh fallback. Every visible body piece
    // here is skull, vertebra, rib, shoulder/arm bone, jaw, hood, or cloak.
    localBox(
        {0.0f, 1.22f, 0.0f},
        {0.10f, 0.56f, 0.10f},
        character::PoseJoint::SpineLower,
        neeshegoShellPalette
    );
    localBox(
        {0.0f, 1.78f, 0.0f},
        {0.11f, 0.62f, 0.11f},
        character::PoseJoint::SpineUpper,
        neeshegoShellPalette
    );
    localBox(
        {0.0f, 2.23f, 0.0f},
        {0.10f, 0.30f, 0.10f},
        character::PoseJoint::Neck,
        neeshegoShellPalette
    );

    // Rib cage as separated bone bars rather than a human torso volume.
    for (int rib = 0; rib < 5; ++rib) {
        const float y = 1.58f + static_cast<float>(rib) * 0.13f;
        const float width = 0.78f - static_cast<float>(rib) * 0.055f;
        localBox(
            {0.0f, y, 0.0f},
            {width, 0.055f, 0.11f},
            character::PoseJoint::SpineUpper,
            neeshegoShellPalette
        );
    }

    localBox(
        {-0.50f, 2.02f, 0.0f},
        {0.46f, 0.09f, 0.10f},
        character::PoseJoint::LeftShoulder,
        neeshegoShellPalette
    );
    localBox(
        {0.50f, 2.02f, 0.0f},
        {0.46f, 0.09f, 0.10f},
        character::PoseJoint::RightShoulder,
        neeshegoShellPalette
    );
    localBox(
        {-0.72f, 1.64f, 0.0f},
        {0.08f, 0.72f, 0.08f},
        character::PoseJoint::LeftUpperArm,
        neeshegoShellPalette
    );
    localBox(
        {0.72f, 1.64f, 0.0f},
        {0.08f, 0.72f, 0.08f},
        character::PoseJoint::RightUpperArm,
        neeshegoShellPalette
    );

    // Skull + jaw. The jaw is a distinct channel so open-jaw dialogue/impact
    // performance is visibly executable instead of a prose-only expression.
    localBox(
        {0.0f, 2.63f, 0.0f},
        {0.58f, 0.54f, 0.50f},
        character::PoseJoint::Skull,
        neeshegoShellPalette
    );
    localBox(
        {0.0f, 2.40f - pose.jawOpen * 0.10f, 0.13f},
        {0.34f, 0.10f + pose.jawOpen * 0.09f, 0.24f},
        character::PoseJoint::Jaw,
        neeshegoShellPalette
    );

    // Canon skull spikes: bone hardware, not hair/horns on a human head.
    for (int spike = -2; spike <= 2; ++spike) {
        const float x = static_cast<float>(spike) * 0.16f;
        const float roll = static_cast<float>(spike) * -0.12f;
        const Mat4 spikeModel = multiply(
            root,
            multiply(
                translation({x, 3.03f - std::fabs(x) * 0.15f, -0.02f}),
                multiply(
                    rotationZ(roll),
                    scale({0.055f, 0.54f + (2 - std::abs(spike)) * 0.06f, 0.055f})
                )
            )
        );
        drawModel(spikeModel, neeshegoShellPalette);
    }

    // Hood/cloak are cloth around exposed bone, never a hidden fleshy torso.
    localBox(
        {0.0f, 2.72f, -0.18f},
        {0.86f, 0.66f, 0.16f},
        character::PoseJoint::Hood,
        neeshegoMidnightPalette
    );
    for (int strip = -2; strip <= 2; ++strip) {
        localBox(
            {static_cast<float>(strip) * 0.22f, 1.18f, -0.18f},
            {0.16f, 1.30f - std::fabs(static_cast<float>(strip)) * 0.12f, 0.09f},
            character::PoseJoint::Cloak,
            neeshegoMidnightPalette
        );
    }
}

inline void neeshegoApplyCharacterPerformancePass(
    const HakuiSceneState& scene,
    const PlayerState& player,
    SDL_GPUCommandBuffer* commands,
    SDL_GPURenderPass* pass,
    SDL_GPUBuffer* cubeVertexBuffer,
    SDL_GPUGraphicsPipeline* opaquePipeline,
    Uint32 cubeVertexCount,
    const math::Mat4& viewProjection
) noexcept
{
    using namespace hakui::math;

    const character::CharacterPerformanceEvent playerEvent =
        neeshegoPlayerPerformanceEvent(scene, player);
    const character::CharacterPoseExecution playerPose =
        character::executeCharacterPerformance(
            player.characterId,
            playerEvent,
            neeshegoPhase01(player.idlePhase + player.gaitPhase)
        );
    const Mat4 playerRoot = multiply(
        translation({player.x, player.y, player.z}),
        rotationY(player.yaw)
    );
    neeshegoDrawPerformanceRig(
        commands,
        pass,
        cubeVertexBuffer,
        opaquePipeline,
        cubeVertexCount,
        viewProjection,
        playerRoot,
        playerPose,
        false
    );

    for (const NpcState& npc : scene.npcs) {
        if (npc.characterId != character::CharacterId::Saelis) continue;
        const character::CharacterPoseExecution npcPose =
            character::executeCharacterPerformance(
                npc.characterId,
                neeshegoNpcPerformanceEvent(npc),
                neeshegoPhase01(npc.idlePhase + npc.gaitPhase)
            );
        const Mat4 npcRoot = multiply(
            translation({npc.x, npc.y, npc.z}),
            rotationY(npc.yaw)
        );
        neeshegoDrawPerformanceRig(
            commands,
            pass,
            cubeVertexBuffer,
            opaquePipeline,
            cubeVertexCount,
            viewProjection,
            npcRoot,
            npcPose,
            false
        );
    }

    // Reaper currently remains an L17 spawned-but-unbound character instance.
    // L22 therefore exposes a native skeletal performance stage only when the
    // developer explicitly selects the Reaper profile. This proves the actual
    // Reaper rig/pose language without inventing simulation transform authority.
    if (neeshegoPreviewCharacter(player) == character::CharacterId::Reaper) {
        const character::CharacterPoseExecution reaperPose =
            character::executeCharacterPerformance(
                character::CharacterId::Reaper,
                character::performanceEventFor(neeshegoRenderEvent(scene)),
                neeshegoPhase01(player.idlePhase + player.gaitPhase)
            );
        const Mat4 reaperRoot = multiply(
            translation({player.x + 2.2f, player.y, player.z}),
            rotationY(player.yaw)
        );
        neeshegoDrawPerformanceRig(
            commands,
            pass,
            cubeVertexBuffer,
            opaquePipeline,
            cubeVertexCount,
            viewProjection,
            reaperRoot,
            reaperPose,
            true
        );
    }
}

// L22 wraps L20's end-of-pass execution. Character pose rigs are drawn first;
// L20 then applies manga scanline/halftone/xerox/panel treatment and performs
// the real SDL_EndGPURenderPass call.
inline void neeshegoEndPerformanceRenderPass(
    const HakuiSceneState& scene,
    const PlayerState& player,
    SDL_GPUCommandBuffer* commands,
    SDL_GPURenderPass* pass,
    SDL_GPUBuffer* cubeVertexBuffer,
    SDL_GPUGraphicsPipeline* opaquePipeline,
    SDL_GPUGraphicsPipeline* glassPipeline,
    Uint32 cubeVertexCount,
    const math::Mat4& viewProjection
) noexcept
{
    neeshegoApplyCharacterPerformancePass(
        scene,
        player,
        commands,
        pass,
        cubeVertexBuffer,
        opaquePipeline,
        cubeVertexCount,
        viewProjection
    );
    neeshegoEndGPURenderPass(
        scene,
        player,
        commands,
        pass,
        cubeVertexBuffer,
        opaquePipeline,
        glassPipeline,
        cubeVertexCount
    );
}

} // namespace hakui::render
