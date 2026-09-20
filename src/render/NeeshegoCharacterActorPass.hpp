#pragma once

#include "character/CharacterActorAuthority.hpp"
#include "render/NeeshegoPerformancePass.hpp"

namespace hakui::render {

[[nodiscard]] inline character::CharacterPerformanceEvent
neeshegoCharacterActorPerformanceEvent(
    const HakuiSceneState& scene,
    const character::CharacterActorState& actor
) noexcept
{
    const character::MangaRenderEvent renderEvent = neeshegoRenderEvent(scene);
    if (renderEvent != character::MangaRenderEvent::Exploration) {
        return character::performanceEventFor(renderEvent);
    }

    switch (actor.activity) {
        case character::CharacterActorActivity::Walking:
            return character::CharacterPerformanceEvent::Locomotion;
        case character::CharacterActorActivity::ObservingPlayer:
            return character::CharacterPerformanceEvent::Dialogue;
        case character::CharacterActorActivity::Idle:
            return character::CharacterPerformanceEvent::Idle;
    }
    return character::CharacterPerformanceEvent::Idle;
}

inline void neeshegoApplyIndependentCharacterActorPass(
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

    // L22's explicit Reaper preview already draws a temporary QA skeleton at
    // the player. Do not double-draw the authoritative L23 actor in that one
    // developer mode. Normal gameplay always uses the authoritative actor.
    if (neeshegoPreviewCharacter(player) == character::CharacterId::Reaper) {
        return;
    }

    for (const character::CharacterActorState& actor :
         character::publishedCharacterActors()) {
        if (!actor.active || actor.characterId == character::CharacterId::None) {
            continue;
        }

        const character::CharacterPoseExecution pose =
            character::executeCharacterPerformance(
                actor.characterId,
                neeshegoCharacterActorPerformanceEvent(scene, actor),
                neeshegoPhase01(actor.idlePhase + actor.gaitPhase)
            );
        if (!pose.active) continue;

        const Mat4 root = multiply(
            translation({actor.x, actor.y, actor.z}),
            rotationY(actor.yaw)
        );
        neeshegoDrawPerformanceRig(
            commands,
            pass,
            cubeVertexBuffer,
            opaquePipeline,
            cubeVertexCount,
            viewProjection,
            root,
            pose,
            pose.skeletalOnly
        );
    }
}

// L23 draws independent authoritative cast members first, then delegates to
// L22 for Player/NPC performance and finally L20's manga overlay/end-pass.
inline void neeshegoEndCharacterActorRenderPass(
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
    neeshegoApplyIndependentCharacterActorPass(
        scene,
        player,
        commands,
        pass,
        cubeVertexBuffer,
        opaquePipeline,
        cubeVertexCount,
        viewProjection
    );
    neeshegoEndPerformanceRenderPass(
        scene,
        player,
        commands,
        pass,
        cubeVertexBuffer,
        opaquePipeline,
        glassPipeline,
        cubeVertexCount,
        viewProjection
    );
}

} // namespace hakui::render
