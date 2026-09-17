#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "avatar/BodyProfile.hpp"
#include "character/CharacterIdentity.hpp"

namespace hakui::character {

enum class CharacterRigArchetype : std::uint8_t {
    Humanoid,
    ReaperSkeleton,
};

enum class CharacterSurfaceLanguage : std::uint8_t {
    RasterVeil,
    SaelisSynthetic,
    ReaperXerox,
};

// L18 immutable embodiment canon.
//
// This profile describes how a canonical character is allowed to acquire a
// body/rig and visual language. It does not own transform, health, movement,
// combat, renderer resources, animation playback, or HOME persistence.
struct CharacterEmbodimentProfile {
    CharacterId characterId = CharacterId::None;
    CharacterRigArchetype rig = CharacterRigArchetype::Humanoid;
    std::optional<avatar::BodyProfileId> bodyProfile;
    CharacterVisualState visualState = CharacterVisualState::RasterVeil;
    CharacterSurfaceLanguage surface = CharacterSurfaceLanguage::RasterVeil;
    std::string_view rigKey;
    std::string_view materialKey;
    std::string_view animationSetKey;
    bool hasHumanFace = true;
    bool hasHair = true;
};

inline constexpr CharacterEmbodimentProfile agnathosEmbodiment{
    CharacterId::Agnathos,
    CharacterRigArchetype::Humanoid,
    avatar::BodyProfileId::Male,
    CharacterVisualState::RasterVeil,
    CharacterSurfaceLanguage::RasterVeil,
    "hakui.humanoid.v1",
    "neeshego.agnathos.raster-veil.bnw",
    "neeshego.agnathos.anim.v1",
    true,
    true,
};

inline constexpr CharacterEmbodimentProfile saelisEmbodiment{
    CharacterId::Saelis,
    CharacterRigArchetype::Humanoid,
    avatar::BodyProfileId::Female,
    CharacterVisualState::SaelisDefault,
    CharacterSurfaceLanguage::SaelisSynthetic,
    "hakui.humanoid.v1",
    "neeshego.saelis.synthetic.bnw",
    "neeshego.saelis.anim.v1",
    true,
    true,
};

inline constexpr CharacterEmbodimentProfile reaperEmbodiment{
    CharacterId::Reaper,
    CharacterRigArchetype::ReaperSkeleton,
    std::nullopt,
    CharacterVisualState::ReaperSkeleton,
    CharacterSurfaceLanguage::ReaperXerox,
    "hakui.reaper-skeleton.v1",
    "neeshego.reaper.xerox.bnw",
    "neeshego.reaper.anim.v1",
    false,
    false,
};

[[nodiscard]] constexpr const CharacterEmbodimentProfile* canonicalEmbodiment(
    CharacterId id
) noexcept
{
    switch (id) {
        case CharacterId::Agnathos:
            return &agnathosEmbodiment;
        case CharacterId::Saelis:
            return &saelisEmbodiment;
        case CharacterId::Reaper:
            return &reaperEmbodiment;
        case CharacterId::None:
            return nullptr;
    }
    return nullptr;
}

} // namespace hakui::character
