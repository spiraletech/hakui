#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string_view>

#include <SDL3/SDL.h>

#include "character/NeeshegoRenderExecution.hpp"
#include "render/DebugWorldRenderer.hpp"
#include "render/Math3D.hpp"

namespace hakui::render {

inline constexpr Uint32 neeshegoShellPalette = 0;
inline constexpr Uint32 neeshegoMidnightPalette = 1;
inline constexpr Uint32 neeshegoVoidPalette = 5;

[[nodiscard]] inline character::CharacterId neeshegoPreviewCharacter(
    const PlayerState& player
) noexcept
{
    static const character::CharacterId overrideCharacter = [] {
        const char* value = SDL_getenv("HAKUI_NEESHEGO_PROFILE");
        if (!value || !*value) return character::CharacterId::None;
        const std::string_view text{value};
        if (text == "agnathos") return character::CharacterId::Agnathos;
        if (text == "saelis") return character::CharacterId::Saelis;
        if (text == "reaper") return character::CharacterId::Reaper;
        return character::CharacterId::None;
    }();
    return overrideCharacter == character::CharacterId::None
        ? player.characterId
        : overrideCharacter;
}

[[nodiscard]] inline character::MangaRenderEvent neeshegoRenderEvent(
    const HakuiSceneState& scene
) noexcept
{
    static const int overrideEvent = [] {
        const char* value = SDL_getenv("HAKUI_NEESHEGO_EVENT");
        if (!value || !*value) return -1;
        const std::string_view text{value};
        if (text == "exploration") return 0;
        if (text == "dialogue") return 1;
        if (text == "windup") return 2;
        if (text == "impact") return 3;
        if (text == "realm") return 4;
        if (text == "overload") return 5;
        return -1;
    }();
    if (overrideEvent >= 0) {
        return static_cast<character::MangaRenderEvent>(overrideEvent);
    }

    if (scene.playerHitPulse > 0.015f || scene.opponentHitPulse > 0.015f) {
        return character::MangaRenderEvent::CombatImpact;
    }
    if (scene.combatActive &&
        scene.playerCombatState == combat::CombatState::Windup) {
        return character::MangaRenderEvent::CombatWindup;
    }
    if (scene.chatInputActive || scene.chatBubbleActive) {
        return character::MangaRenderEvent::Dialogue;
    }
    return character::MangaRenderEvent::Exploration;
}

[[nodiscard]] inline character::NeeshegoRenderExecution neeshegoExecution(
    const HakuiSceneState& scene,
    const PlayerState& player
) noexcept
{
    return character::executeMangaEvent(
        neeshegoPreviewCharacter(player),
        neeshegoRenderEvent(scene)
    );
}

[[nodiscard]] inline Uint32 neeshegoRemapPalette(
    Uint32 source,
    character::MangaPaletteExecution mode
) noexcept
{
    if (source == neeshegoVoidPalette) return neeshegoVoidPalette;

    switch (mode) {
        case character::MangaPaletteExecution::Preserve:
            return source;
        case character::MangaPaletteExecution::RasterMonochrome:
            switch (source) {
                case 1:
                case 3:
                case 7:
                    return neeshegoMidnightPalette;
                default:
                    return neeshegoShellPalette;
            }
        case character::MangaPaletteExecution::SyntheticMonochrome:
            return source == 1 || source == 7
                ? neeshegoMidnightPalette
                : neeshegoShellPalette;
        case character::MangaPaletteExecution::XeroxMonochrome:
            switch (source) {
                case 1:
                case 3:
                case 5:
                case 7:
                    return neeshegoVoidPalette;
                default:
                    return neeshegoShellPalette;
            }
    }
    return source;
}

// Called by the L20 native renderer shim for the two cube-batch draw sites in
// DebugWorldRenderer. The original geometry, transforms and draw order remain
// untouched; only the palette block is remapped to the active Neeshego ink
// language. This keeps simulation and renderer authority separated.
inline void neeshegoDrawGPUPrimitives(
    const HakuiSceneState& scene,
    const PlayerState& player,
    SDL_GPURenderPass* pass,
    Uint32 vertexCount,
    Uint32 instanceCount,
    Uint32 firstVertex,
    Uint32 firstInstance
) noexcept
{
    const character::NeeshegoRenderExecution execution =
        neeshegoExecution(scene, player);
    if (execution.active && execution.monochrome && vertexCount > 0 &&
        firstVertex % vertexCount == 0) {
        const Uint32 sourcePalette = firstVertex / vertexCount;
        if (sourcePalette < 8) {
            firstVertex = neeshegoRemapPalette(
                sourcePalette, execution.palette
            ) * vertexCount;
        }
    }
    SDL_DrawGPUPrimitives(
        pass, vertexCount, instanceCount, firstVertex, firstInstance
    );
}

[[nodiscard]] inline std::uint32_t neeshegoHash(std::uint32_t value) noexcept
{
    value ^= value >> 16;
    value *= 0x7feb352du;
    value ^= value >> 15;
    value *= 0x846ca68bu;
    value ^= value >> 16;
    return value;
}

[[nodiscard]] inline float neeshegoUnit(std::uint32_t value) noexcept
{
    return static_cast<float>(neeshegoHash(value) & 0xffffu) / 65535.0f;
}

[[nodiscard]] inline const char* neeshegoCharacterLabel(
    character::CharacterId id
) noexcept
{
    switch (id) {
        case character::CharacterId::Agnathos: return "AGNATHOS";
        case character::CharacterId::Saelis: return "SAELIS";
        case character::CharacterId::Reaper: return "THE REAPER";
        case character::CharacterId::None: return "NONE";
    }
    return "NONE";
}

[[nodiscard]] inline const char* neeshegoEventLabel(
    character::MangaRenderEvent event
) noexcept
{
    switch (event) {
        case character::MangaRenderEvent::Exploration: return "EXPLORATION";
        case character::MangaRenderEvent::Dialogue: return "DIALOGUE";
        case character::MangaRenderEvent::CombatWindup: return "COMBAT WINDUP";
        case character::MangaRenderEvent::CombatImpact: return "COMBAT IMPACT";
        case character::MangaRenderEvent::RealmShift: return "REALM SHIFT";
        case character::MangaRenderEvent::Overload: return "OVERLOAD";
    }
    return "UNKNOWN";
}

// Final pass overlays are intentionally primitive and deterministic. They use
// the existing first-party cube vertex buffer/pipelines so L20 does not add a
// second renderer or shader authority. A later shader pass can replace these
// primitives while preserving the same execution-plan contract.
inline void neeshegoEndGPURenderPass(
    const HakuiSceneState& scene,
    const PlayerState& player,
    SDL_GPUCommandBuffer* commands,
    SDL_GPURenderPass* pass,
    SDL_GPUBuffer* cubeVertexBuffer,
    SDL_GPUGraphicsPipeline* opaquePipeline,
    SDL_GPUGraphicsPipeline* glassPipeline,
    Uint32 cubeVertexCount
) noexcept
{
    using namespace hakui::math;

    static thread_local std::uint32_t frameCounter = 0;
    ++frameCounter;

    const character::MangaRenderEvent event = neeshegoRenderEvent(scene);
    const character::NeeshegoRenderExecution execution =
        neeshegoExecution(scene, player);

    if (execution.active && commands && pass && cubeVertexBuffer &&
        opaquePipeline && glassPipeline && cubeVertexCount > 0) {
        SDL_GPUBufferBinding binding{};
        binding.buffer = cubeVertexBuffer;
        binding.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 0, &binding, 1);
        SDL_BindGPUGraphicsPipeline(pass, glassPipeline);

        const auto drawClip = [&](float x,
                                  float y,
                                  float width,
                                  float height,
                                  float rotation,
                                  Uint32 palette,
                                  float alpha,
                                  float z = 0.001f) {
            const float opacity = std::clamp(alpha, 0.0f, 1.0f);
            if (opacity <= 0.0001f || width <= 0.0f || height <= 0.0f) {
                return;
            }
            SDL_SetGPUBlendConstants(
                pass, SDL_FColor{opacity, opacity, opacity, opacity}
            );
            const Mat4 clipModel = multiply(
                translation({x, y, z}),
                multiply(rotationZ(rotation), scale({width, height, 0.002f}))
            );
            SDL_PushGPUVertexUniformData(
                commands, 0, clipModel.m, sizeof(clipModel.m)
            );
            SDL_DrawGPUPrimitives(
                pass,
                cubeVertexCount,
                1,
                palette * cubeVertexCount,
                0
            );
        };

        // Black-field crush. Existing world geometry stays intact underneath.
        drawClip(
            0.0f, 0.0f, 2.0f, 2.0f, 0.0f,
            neeshegoVoidPalette, execution.blackWashAlpha, 0.0008f
        );

        // Raster Veil / synthetic scanline layer.
        if (execution.scanlineCount > 0) {
            const float spacing = 2.0f /
                static_cast<float>(execution.scanlineCount);
            for (std::uint16_t i = 0; i < execution.scanlineCount; ++i) {
                const float y = -1.0f + spacing * (static_cast<float>(i) + 0.5f);
                drawClip(
                    0.0f, y, 2.0f, 0.0035f, 0.0f,
                    neeshegoVoidPalette, execution.scanlineAlpha, 0.0007f
                );
            }
        }

        // Sparse halftone. Dot placement is fixed in clip space so camera and
        // gameplay determinism never depend on presentation noise.
        for (std::uint16_t i = 0; i < execution.halftoneDotCount; ++i) {
            const std::uint32_t seed = static_cast<std::uint32_t>(i) * 977u + 31u;
            const float x = -0.96f + neeshegoUnit(seed) * 1.92f;
            const float y = -0.96f + neeshegoUnit(seed + 17u) * 1.92f;
            const float size = 0.004f + neeshegoUnit(seed + 29u) * 0.009f;
            drawClip(
                x, y, size, size, 0.0f,
                neeshegoVoidPalette, execution.halftoneAlpha, 0.0006f
            );
        }

        // Xerox streaks deliberately evolve frame-to-frame, but the pattern is
        // presentation-only and never feeds simulation state back into HAKUI.
        for (std::uint16_t i = 0; i < execution.xeroxStreakCount; ++i) {
            const std::uint32_t seed = frameCounter * 131u +
                static_cast<std::uint32_t>(i) * 811u;
            const float y = -0.96f + neeshegoUnit(seed) * 1.92f;
            const float x = -0.70f + neeshegoUnit(seed + 7u) * 1.40f;
            const float width = 0.18f + neeshegoUnit(seed + 13u) * 1.15f;
            const float height = 0.003f + neeshegoUnit(seed + 19u) * 0.014f;
            const Uint32 palette = (i & 1u) == 0u
                ? neeshegoShellPalette
                : neeshegoVoidPalette;
            drawClip(
                x, y, width, height, 0.0f,
                palette, execution.xeroxAlpha, 0.0005f
            );
        }

        // Manga speed lines converge toward the frame center without modifying
        // the camera. They are strongest on impact frames.
        for (std::uint16_t i = 0; i < execution.speedLineCount; ++i) {
            const float t = static_cast<float>(i) /
                static_cast<float>(std::max<std::uint16_t>(1, execution.speedLineCount));
            const float angle = t * 6.28318530718f;
            const float radius = 0.82f + 0.10f * neeshegoUnit(i + 101u);
            const float x = std::cos(angle) * radius;
            const float y = std::sin(angle) * radius;
            drawClip(
                x, y, 0.42f, 0.005f, angle,
                neeshegoShellPalette, execution.speedLineAlpha, 0.0004f
            );
        }

        if (execution.frameTear) {
            for (std::uint16_t i = 0; i < execution.tearBandCount; ++i) {
                const std::uint32_t seed = frameCounter * 43u + i * 271u;
                const float y = -0.88f + neeshegoUnit(seed) * 1.76f;
                const float shift = (neeshegoUnit(seed + 11u) - 0.5f) * 0.24f;
                const float h = 0.010f + neeshegoUnit(seed + 23u) * 0.026f;
                drawClip(
                    shift, y, 1.92f, h, 0.0f,
                    (i & 1u) == 0u ? neeshegoVoidPalette : neeshegoShellPalette,
                    0.10f + execution.xeroxAlpha * 0.55f,
                    0.0003f
                );
            }
        }

        if (execution.panelBars) {
            drawClip(-0.93f, 0.0f, 0.14f, 2.0f, 0.0f,
                     neeshegoVoidPalette, 0.78f, 0.00025f);
            drawClip(0.93f, 0.0f, 0.14f, 2.0f, 0.0f,
                     neeshegoVoidPalette, 0.78f, 0.00025f);
        }

        if (execution.panelBorder) {
            drawClip(0.0f, 0.985f, 2.0f, 0.026f, 0.0f,
                     neeshegoShellPalette, 0.82f, 0.0002f);
            drawClip(0.0f, -0.985f, 2.0f, 0.026f, 0.0f,
                     neeshegoShellPalette, 0.82f, 0.0002f);
            drawClip(-0.985f, 0.0f, 0.026f, 2.0f, 0.0f,
                     neeshegoShellPalette, 0.82f, 0.0002f);
            drawClip(0.985f, 0.0f, 0.026f, 2.0f, 0.0f,
                     neeshegoShellPalette, 0.82f, 0.0002f);
        }

        if (execution.impactFlashAlpha > 0.0f) {
            drawClip(
                0.0f, 0.0f, 2.0f, 2.0f, 0.0f,
                neeshegoShellPalette, execution.impactFlashAlpha, 0.0001f
            );
        }

        SDL_BindGPUGraphicsPipeline(pass, opaquePipeline);
    }

    static thread_local character::CharacterId lastCharacter =
        character::CharacterId::None;
    static thread_local int lastEvent = -1;
    const character::CharacterId activeCharacter = neeshegoPreviewCharacter(player);
    const int eventValue = static_cast<int>(event);
    if (activeCharacter != lastCharacter || eventValue != lastEvent) {
        SDL_Log(
            "[NEESHEGO] render execution // %s // %s // scan %u // xerox %u // speed %u",
            neeshegoCharacterLabel(activeCharacter),
            neeshegoEventLabel(event),
            static_cast<unsigned>(execution.scanlineCount),
            static_cast<unsigned>(execution.xeroxStreakCount),
            static_cast<unsigned>(execution.speedLineCount)
        );
        lastCharacter = activeCharacter;
        lastEvent = eventValue;
    }

    SDL_EndGPURenderPass(pass);
}

} // namespace hakui::render
