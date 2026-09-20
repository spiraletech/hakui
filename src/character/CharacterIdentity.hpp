#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

#include "avatar/BodyProfile.hpp"

namespace hakui::character {

enum class CharacterId : std::uint32_t {
    None = 0,
    Agnathos = 0x1001,
    Saelis = 0x1002,
    Reaper = 0x1003,
};

enum class CharacterSpecies : std::uint8_t {
    Human,
    Synthetic,
    ReaperSkeleton,
};

enum class CharacterFaction : std::uint8_t {
    SpiralGuardian,
    SpiralCompanion,
    Reaper,
};

enum class RealmAlignment : std::uint8_t {
    SpiralGrove,
    SpiralOS,
    DeathBoundary,
};

enum class CharacterVisualState : std::uint8_t {
    RasterVeil,
    SaelisDefault,
    ReaperSkeleton,
};

enum class EmbodimentClass : std::uint8_t {
    Humanoid,
    NonHumanSkeleton,
};

// L16 immutable character identity.
//
// Identity is canon, not mutable simulation state. A CharacterId may be stored
// on a player or NPC, but position, health, relationships, animation state and
// other runtime values remain owned by their established gameplay authorities.
struct CharacterIdentity {
    CharacterId id = CharacterId::None;
    std::string_view key;
    std::string_view canonicalName;
    CharacterSpecies species = CharacterSpecies::Human;
    CharacterFaction faction = CharacterFaction::SpiralGuardian;
    RealmAlignment realmAlignment = RealmAlignment::SpiralOS;
    std::optional<avatar::BodyProfileId> baseBodyProfile;
    std::string_view faceIdentity;
    CharacterVisualState defaultVisualState = CharacterVisualState::RasterVeil;
    EmbodimentClass embodiment = EmbodimentClass::Humanoid;
    bool playable = false;
};

inline constexpr CharacterIdentity agnathosIdentity{
    CharacterId::Agnathos,
    "agnathos",
    "AGNATHOS",
    CharacterSpecies::Human,
    CharacterFaction::SpiralGuardian,
    RealmAlignment::SpiralGrove,
    avatar::BodyProfileId::Male,
    "agnathos.base-face.v1",
    CharacterVisualState::RasterVeil,
    EmbodimentClass::Humanoid,
    true,
};

inline constexpr CharacterIdentity saelisIdentity{
    CharacterId::Saelis,
    "saelis",
    "SAELIS",
    CharacterSpecies::Synthetic,
    CharacterFaction::SpiralCompanion,
    RealmAlignment::SpiralOS,
    avatar::BodyProfileId::Female,
    "saelis.base-face.v1",
    CharacterVisualState::SaelisDefault,
    EmbodimentClass::Humanoid,
    false,
};

inline constexpr CharacterIdentity reaperIdentity{
    CharacterId::Reaper,
    "reaper",
    "THE REAPER",
    CharacterSpecies::ReaperSkeleton,
    CharacterFaction::Reaper,
    RealmAlignment::DeathBoundary,
    std::nullopt,
    "none.skeletal",
    CharacterVisualState::ReaperSkeleton,
    EmbodimentClass::NonHumanSkeleton,
    false,
};

[[nodiscard]] constexpr const CharacterIdentity* canonicalIdentity(
    CharacterId id
) noexcept
{
    switch (id) {
        case CharacterId::Agnathos:
            return &agnathosIdentity;
        case CharacterId::Saelis:
            return &saelisIdentity;
        case CharacterId::Reaper:
            return &reaperIdentity;
        case CharacterId::None:
            return nullptr;
    }
    return nullptr;
}

[[nodiscard]] constexpr const CharacterIdentity* canonicalIdentity(
    std::string_view key
) noexcept
{
    if (key == agnathosIdentity.key) return &agnathosIdentity;
    if (key == saelisIdentity.key) return &saelisIdentity;
    if (key == reaperIdentity.key) return &reaperIdentity;
    return nullptr;
}

} // namespace hakui::character
