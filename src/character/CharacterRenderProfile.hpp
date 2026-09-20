#pragma once

#include <cstdint>
#include <string_view>

#include "character/CharacterIdentity.hpp"

namespace hakui::character {

enum class MangaInkMode : std::uint8_t {
    CleanInk,
    RasterInk,
    SyntheticInk,
    XeroxCrush,
};

enum class MangaPanelMode : std::uint8_t {
    World,
    PortraitCut,
    ImpactPanel,
    RealmBreak,
};

enum class MangaRenderEvent : std::uint8_t {
    Exploration,
    Dialogue,
    CombatWindup,
    CombatImpact,
    RealmShift,
    Overload,
};

// L19 immutable Neeshego render grammar.
//
// The profile is presentation canon only. It does not own gameplay state,
// transforms, animation playback, GPU resources, shaders, cameras, or HOME
// persistence. Render backends consume the resulting frame treatment.
struct CharacterRenderProfile {
    CharacterId characterId = CharacterId::None;
    std::string_view key;
    MangaInkMode inkMode = MangaInkMode::CleanInk;
    bool monochromeOnly = true;
    bool whiteSubjectOnBlack = false;
    float edgeInk = 0.0f;
    float halftone = 0.0f;
    float scanlines = 0.0f;
    float xeroxNoise = 0.0f;
    float blackField = 0.0f;
    bool allowPortraitCuts = false;
    bool allowSpeedLines = false;
    bool allowFrameTear = false;
};

struct MangaFrameTreatment {
    CharacterId characterId = CharacterId::None;
    MangaPanelMode panel = MangaPanelMode::World;
    bool monochrome = true;
    bool whiteSubjectOnBlack = false;
    bool frameTear = false;
    float edgeInk = 0.0f;
    float halftone = 0.0f;
    float scanlines = 0.0f;
    float xeroxNoise = 0.0f;
    float speedLines = 0.0f;
    float blackField = 0.0f;
    float freezeFrame = 0.0f;
};

[[nodiscard]] constexpr float mangaClamp01(float value) noexcept
{
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

inline constexpr CharacterRenderProfile agnathosRenderProfile{
    CharacterId::Agnathos,
    "neeshego.agnathos.raster-veil.v1",
    MangaInkMode::RasterInk,
    true,
    true,
    0.92f,
    0.42f,
    0.82f,
    0.10f,
    0.88f,
    true,
    true,
    true,
};

inline constexpr CharacterRenderProfile saelisRenderProfile{
    CharacterId::Saelis,
    "neeshego.saelis.synthetic-ink.v1",
    MangaInkMode::SyntheticInk,
    true,
    true,
    0.76f,
    0.58f,
    0.24f,
    0.04f,
    0.72f,
    true,
    true,
    false,
};

inline constexpr CharacterRenderProfile reaperRenderProfile{
    CharacterId::Reaper,
    "neeshego.reaper.xerox-crush.v1",
    MangaInkMode::XeroxCrush,
    true,
    true,
    1.00f,
    0.18f,
    0.06f,
    0.96f,
    0.98f,
    true,
    true,
    true,
};

[[nodiscard]] constexpr const CharacterRenderProfile* canonicalRenderProfile(
    CharacterId id
) noexcept
{
    switch (id) {
        case CharacterId::Agnathos:
            return &agnathosRenderProfile;
        case CharacterId::Saelis:
            return &saelisRenderProfile;
        case CharacterId::Reaper:
            return &reaperRenderProfile;
        case CharacterId::None:
            return nullptr;
    }
    return nullptr;
}

[[nodiscard]] constexpr MangaFrameTreatment composeMangaFrame(
    CharacterId characterId,
    MangaRenderEvent event
) noexcept
{
    const CharacterRenderProfile* profile = canonicalRenderProfile(characterId);
    if (!profile) return {};

    MangaFrameTreatment frame{
        profile->characterId,
        MangaPanelMode::World,
        profile->monochromeOnly,
        profile->whiteSubjectOnBlack,
        false,
        profile->edgeInk,
        profile->halftone,
        profile->scanlines,
        profile->xeroxNoise,
        0.0f,
        profile->blackField,
        0.0f,
    };

    switch (event) {
        case MangaRenderEvent::Exploration:
            break;

        case MangaRenderEvent::Dialogue:
            if (profile->allowPortraitCuts) {
                frame.panel = MangaPanelMode::PortraitCut;
                frame.blackField = mangaClamp01(frame.blackField + 0.06f);
            }
            break;

        case MangaRenderEvent::CombatWindup:
            if (profile->allowSpeedLines) frame.speedLines = 0.52f;
            frame.edgeInk = mangaClamp01(frame.edgeInk + 0.04f);
            break;

        case MangaRenderEvent::CombatImpact:
            frame.panel = MangaPanelMode::ImpactPanel;
            if (profile->allowSpeedLines) frame.speedLines = 1.0f;
            frame.edgeInk = mangaClamp01(frame.edgeInk + 0.10f);
            frame.blackField = 1.0f;
            frame.freezeFrame = 0.84f;
            frame.frameTear = profile->allowFrameTear;
            break;

        case MangaRenderEvent::RealmShift:
            frame.panel = MangaPanelMode::RealmBreak;
            frame.frameTear = profile->allowFrameTear;
            frame.scanlines = mangaClamp01(frame.scanlines + 0.18f);
            frame.xeroxNoise = mangaClamp01(frame.xeroxNoise + 0.12f);
            frame.blackField = mangaClamp01(frame.blackField + 0.08f);
            break;

        case MangaRenderEvent::Overload:
            frame.panel = MangaPanelMode::RealmBreak;
            frame.frameTear = profile->allowFrameTear;
            frame.edgeInk = 1.0f;
            frame.scanlines = mangaClamp01(frame.scanlines + 0.28f);
            frame.xeroxNoise = mangaClamp01(frame.xeroxNoise + 0.24f);
            frame.blackField = 1.0f;
            frame.freezeFrame = 0.48f;
            if (profile->allowSpeedLines) frame.speedLines = 0.68f;
            break;
    }

    return frame;
}

} // namespace hakui::character
