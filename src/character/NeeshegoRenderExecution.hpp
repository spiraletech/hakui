#pragma once

#include <cstdint>

#include "character/CharacterRenderProfile.hpp"

namespace hakui::character {

enum class MangaPaletteExecution : std::uint8_t {
    Preserve,
    RasterMonochrome,
    SyntheticMonochrome,
    XeroxMonochrome,
};

// L20 concrete renderer execution plan.
//
// L19 describes the canonical manga frame treatment. L20 converts that canon
// into bounded, backend-ready execution parameters. This remains pure data:
// no SDL/GPU objects, gameplay state, camera state, animation state, or HOME
// persistence is owned here.
struct NeeshegoRenderExecution {
    CharacterId characterId = CharacterId::None;
    MangaPanelMode panel = MangaPanelMode::World;
    MangaInkMode inkMode = MangaInkMode::CleanInk;
    MangaPaletteExecution palette = MangaPaletteExecution::Preserve;
    bool active = false;
    bool monochrome = false;
    bool whiteSubjectOnBlack = false;
    bool frameTear = false;

    float blackWashAlpha = 0.0f;
    float edgeInk = 0.0f;
    float scanlineAlpha = 0.0f;
    float halftoneAlpha = 0.0f;
    float xeroxAlpha = 0.0f;
    float speedLineAlpha = 0.0f;
    float impactFlashAlpha = 0.0f;

    std::uint16_t scanlineCount = 0;
    std::uint16_t halftoneDotCount = 0;
    std::uint16_t xeroxStreakCount = 0;
    std::uint16_t speedLineCount = 0;
    std::uint16_t tearBandCount = 0;
    bool panelBars = false;
    bool panelBorder = false;
};

[[nodiscard]] constexpr std::uint16_t mangaCount(
    float weight,
    std::uint16_t minimum,
    std::uint16_t maximum
) noexcept
{
    const float clamped = mangaClamp01(weight);
    const float span = static_cast<float>(maximum - minimum);
    return static_cast<std::uint16_t>(
        static_cast<float>(minimum) + clamped * span + 0.5f
    );
}

[[nodiscard]] constexpr MangaPaletteExecution mangaPaletteExecution(
    MangaInkMode mode
) noexcept
{
    switch (mode) {
        case MangaInkMode::RasterInk:
            return MangaPaletteExecution::RasterMonochrome;
        case MangaInkMode::SyntheticInk:
            return MangaPaletteExecution::SyntheticMonochrome;
        case MangaInkMode::XeroxCrush:
            return MangaPaletteExecution::XeroxMonochrome;
        case MangaInkMode::CleanInk:
            return MangaPaletteExecution::Preserve;
    }
    return MangaPaletteExecution::Preserve;
}

[[nodiscard]] constexpr NeeshegoRenderExecution executeMangaFrame(
    const MangaFrameTreatment& frame
) noexcept
{
    const CharacterRenderProfile* profile =
        canonicalRenderProfile(frame.characterId);
    if (!profile) return {};

    NeeshegoRenderExecution execution{};
    execution.characterId = frame.characterId;
    execution.panel = frame.panel;
    execution.inkMode = profile->inkMode;
    execution.palette = mangaPaletteExecution(profile->inkMode);
    execution.active = true;
    execution.monochrome = frame.monochrome;
    execution.whiteSubjectOnBlack = frame.whiteSubjectOnBlack;
    execution.frameTear = frame.frameTear;

    // Keep the base world readable. The black field is expressed as a bounded
    // translucent wash rather than replacing scene visibility outright.
    execution.blackWashAlpha = mangaClamp01(0.025f + frame.blackField * 0.16f);
    execution.edgeInk = mangaClamp01(frame.edgeInk);
    execution.scanlineAlpha = mangaClamp01(frame.scanlines * 0.26f);
    execution.halftoneAlpha = mangaClamp01(frame.halftone * 0.11f);
    execution.xeroxAlpha = mangaClamp01(frame.xeroxNoise * 0.24f);
    execution.speedLineAlpha = mangaClamp01(frame.speedLines * 0.23f);
    execution.impactFlashAlpha = mangaClamp01(frame.freezeFrame * 0.16f);

    execution.scanlineCount = frame.scanlines > 0.001f
        ? mangaCount(frame.scanlines, 12, 48)
        : 0;
    execution.halftoneDotCount = frame.halftone > 0.001f
        ? mangaCount(frame.halftone, 12, 56)
        : 0;
    execution.xeroxStreakCount = frame.xeroxNoise > 0.001f
        ? mangaCount(frame.xeroxNoise, 4, 28)
        : 0;
    execution.speedLineCount = frame.speedLines > 0.001f
        ? mangaCount(frame.speedLines, 6, 22)
        : 0;
    execution.tearBandCount = frame.frameTear
        ? mangaCount(frame.xeroxNoise + frame.scanlines * 0.35f, 2, 8)
        : 0;

    execution.panelBars = frame.panel == MangaPanelMode::PortraitCut;
    execution.panelBorder = frame.panel == MangaPanelMode::ImpactPanel ||
                            frame.panel == MangaPanelMode::RealmBreak;
    return execution;
}

[[nodiscard]] constexpr NeeshegoRenderExecution executeMangaEvent(
    CharacterId characterId,
    MangaRenderEvent event
) noexcept
{
    return executeMangaFrame(composeMangaFrame(characterId, event));
}

} // namespace hakui::character
