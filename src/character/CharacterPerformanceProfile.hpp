#pragma once

#include <cstdint>
#include <string_view>

#include "character/CharacterIdentity.hpp"
#include "character/CharacterRenderProfile.hpp"

namespace hakui::character {

enum class CharacterMotionGrammar : std::uint8_t {
    GuardianStillness,
    SyntheticPrecision,
    ReaperStaccato,
};

enum class CharacterExpression : std::uint8_t {
    Neutral,
    RasterBlank,
    RasterFocused,
    RasterStrained,
    SyntheticNeutral,
    SyntheticWarm,
    SyntheticAlert,
    ReaperStill,
    ReaperOpenJaw,
    ReaperRattle,
};

enum class CharacterPerformanceEvent : std::uint8_t {
    Idle,
    Locomotion,
    Dialogue,
    CombatReady,
    CombatImpact,
    RealmShift,
    Overload,
};

// Immutable authored motion/expression language. This describes the character,
// not a mutable animation state machine. Runtime animation playback, root
// motion, transforms, combat and renderer resources remain in their existing
// authorities.
struct CharacterPerformanceProfile {
    CharacterId characterId = CharacterId::None;
    std::string_view key;
    CharacterMotionGrammar motion = CharacterMotionGrammar::GuardianStillness;
    CharacterExpression defaultExpression = CharacterExpression::Neutral;

    float idleAmplitude = 0.0f;
    float strideScale = 1.0f;
    float armCounterSwing = 1.0f;
    float gestureAmplitude = 1.0f;
    float poseSnap = 0.0f;
    float asymmetry = 0.0f;
    float phaseDiscontinuity = 0.0f;

    bool usesHumanFacialMuscles = true;
    bool usesJawChannel = true;
    bool usesRasterEyeChannel = false;
};

// Deterministic performance sample consumed by an animation/pose backend.
// Values are bounded authored offsets/weights, never gameplay authority.
struct CharacterPerformanceFrame {
    CharacterId characterId = CharacterId::None;
    CharacterMotionGrammar motion = CharacterMotionGrammar::GuardianStillness;
    CharacterExpression expression = CharacterExpression::Neutral;

    float headPitch = 0.0f;
    float headRoll = 0.0f;
    float torsoPitch = 0.0f;
    float torsoRoll = 0.0f;
    float leftArmBias = 0.0f;
    float rightArmBias = 0.0f;
    float strideScale = 1.0f;
    float idleAmplitude = 0.0f;
    float gestureWeight = 0.0f;
    float jawOpen = 0.0f;
    float cranialIntensity = 0.0f;
    float snapWeight = 0.0f;
    float staccatoWeight = 0.0f;
};

[[nodiscard]] constexpr float performanceClamp(float value, float lo, float hi) noexcept
{
    return value < lo ? lo : (value > hi ? hi : value);
}

inline constexpr CharacterPerformanceProfile agnathosPerformanceProfile{
    CharacterId::Agnathos,
    "neeshego.agnathos.performance.v1",
    CharacterMotionGrammar::GuardianStillness,
    CharacterExpression::RasterBlank,
    0.34f,
    0.94f,
    0.72f,
    0.78f,
    0.18f,
    0.22f,
    0.08f,
    true,
    true,
    true,
};

inline constexpr CharacterPerformanceProfile saelisPerformanceProfile{
    CharacterId::Saelis,
    "neeshego.saelis.performance.v1",
    CharacterMotionGrammar::SyntheticPrecision,
    CharacterExpression::SyntheticNeutral,
    0.18f,
    0.90f,
    0.54f,
    0.62f,
    0.42f,
    0.08f,
    0.02f,
    true,
    true,
    false,
};

inline constexpr CharacterPerformanceProfile reaperPerformanceProfile{
    CharacterId::Reaper,
    "neeshego.reaper.performance.v1",
    CharacterMotionGrammar::ReaperStaccato,
    CharacterExpression::ReaperStill,
    0.06f,
    0.78f,
    0.16f,
    0.38f,
    0.92f,
    0.72f,
    0.88f,
    false,
    true,
    false,
};

[[nodiscard]] constexpr const CharacterPerformanceProfile* canonicalPerformanceProfile(
    CharacterId id
) noexcept
{
    switch (id) {
        case CharacterId::Agnathos: return &agnathosPerformanceProfile;
        case CharacterId::Saelis: return &saelisPerformanceProfile;
        case CharacterId::Reaper: return &reaperPerformanceProfile;
        case CharacterId::None: return nullptr;
    }
    return nullptr;
}

[[nodiscard]] constexpr CharacterPerformanceEvent performanceEventFor(
    MangaRenderEvent event
) noexcept
{
    switch (event) {
        case MangaRenderEvent::Exploration: return CharacterPerformanceEvent::Idle;
        case MangaRenderEvent::Dialogue: return CharacterPerformanceEvent::Dialogue;
        case MangaRenderEvent::CombatWindup: return CharacterPerformanceEvent::CombatReady;
        case MangaRenderEvent::CombatImpact: return CharacterPerformanceEvent::CombatImpact;
        case MangaRenderEvent::RealmShift: return CharacterPerformanceEvent::RealmShift;
        case MangaRenderEvent::Overload: return CharacterPerformanceEvent::Overload;
    }
    return CharacterPerformanceEvent::Idle;
}

[[nodiscard]] constexpr CharacterPerformanceFrame composeCharacterPerformance(
    CharacterId characterId,
    CharacterPerformanceEvent event
) noexcept
{
    const CharacterPerformanceProfile* profile = canonicalPerformanceProfile(characterId);
    if (!profile) return {};

    CharacterPerformanceFrame frame{};
    frame.characterId = characterId;
    frame.motion = profile->motion;
    frame.expression = profile->defaultExpression;
    frame.strideScale = profile->strideScale;
    frame.idleAmplitude = profile->idleAmplitude;
    frame.gestureWeight = profile->gestureAmplitude;
    frame.snapWeight = profile->poseSnap;
    frame.staccatoWeight = profile->phaseDiscontinuity;

    switch (characterId) {
        case CharacterId::Agnathos:
            switch (event) {
                case CharacterPerformanceEvent::Idle:
                    frame.headPitch = 0.03f;
                    frame.torsoRoll = -0.015f;
                    frame.cranialIntensity = 0.52f;
                    break;
                case CharacterPerformanceEvent::Locomotion:
                    frame.headPitch = -0.02f;
                    frame.torsoPitch = 0.025f;
                    frame.cranialIntensity = 0.58f;
                    break;
                case CharacterPerformanceEvent::Dialogue:
                    frame.expression = CharacterExpression::RasterFocused;
                    frame.headRoll = -0.07f;
                    frame.leftArmBias = 0.10f;
                    frame.gestureWeight = 0.86f;
                    frame.cranialIntensity = 0.68f;
                    break;
                case CharacterPerformanceEvent::CombatReady:
                    frame.expression = CharacterExpression::RasterFocused;
                    frame.headPitch = -0.055f;
                    frame.torsoPitch = 0.09f;
                    frame.leftArmBias = -0.10f;
                    frame.rightArmBias = -0.16f;
                    frame.cranialIntensity = 0.78f;
                    break;
                case CharacterPerformanceEvent::CombatImpact:
                    frame.expression = CharacterExpression::RasterStrained;
                    frame.headRoll = 0.12f;
                    frame.torsoRoll = -0.10f;
                    frame.cranialIntensity = 0.92f;
                    frame.snapWeight = 0.74f;
                    break;
                case CharacterPerformanceEvent::RealmShift:
                    frame.expression = CharacterExpression::RasterFocused;
                    frame.headPitch = 0.10f;
                    frame.torsoPitch = -0.06f;
                    frame.cranialIntensity = 1.0f;
                    frame.snapWeight = 0.56f;
                    break;
                case CharacterPerformanceEvent::Overload:
                    frame.expression = CharacterExpression::RasterStrained;
                    frame.headPitch = 0.18f;
                    frame.headRoll = -0.11f;
                    frame.torsoPitch = 0.12f;
                    frame.torsoRoll = 0.08f;
                    frame.leftArmBias = 0.20f;
                    frame.rightArmBias = -0.16f;
                    frame.cranialIntensity = 1.0f;
                    frame.snapWeight = 0.88f;
                    break;
            }
            break;

        case CharacterId::Saelis:
            switch (event) {
                case CharacterPerformanceEvent::Idle:
                    frame.headRoll = 0.015f;
                    frame.cranialIntensity = 0.30f;
                    break;
                case CharacterPerformanceEvent::Locomotion:
                    frame.torsoPitch = 0.015f;
                    frame.cranialIntensity = 0.34f;
                    break;
                case CharacterPerformanceEvent::Dialogue:
                    frame.expression = CharacterExpression::SyntheticWarm;
                    frame.headRoll = 0.055f;
                    frame.leftArmBias = 0.08f;
                    frame.rightArmBias = -0.04f;
                    frame.gestureWeight = 0.74f;
                    frame.cranialIntensity = 0.44f;
                    break;
                case CharacterPerformanceEvent::CombatReady:
                    frame.expression = CharacterExpression::SyntheticAlert;
                    frame.headPitch = -0.035f;
                    frame.torsoPitch = 0.055f;
                    frame.gestureWeight = 0.38f;
                    frame.cranialIntensity = 0.62f;
                    break;
                case CharacterPerformanceEvent::CombatImpact:
                    frame.expression = CharacterExpression::SyntheticAlert;
                    frame.headRoll = -0.055f;
                    frame.torsoRoll = 0.045f;
                    frame.snapWeight = 0.66f;
                    frame.cranialIntensity = 0.70f;
                    break;
                case CharacterPerformanceEvent::RealmShift:
                    frame.expression = CharacterExpression::SyntheticAlert;
                    frame.headPitch = 0.055f;
                    frame.cranialIntensity = 0.76f;
                    frame.snapWeight = 0.70f;
                    break;
                case CharacterPerformanceEvent::Overload:
                    frame.expression = CharacterExpression::SyntheticAlert;
                    frame.headPitch = 0.08f;
                    frame.torsoRoll = -0.055f;
                    frame.cranialIntensity = 0.84f;
                    frame.snapWeight = 0.82f;
                    break;
            }
            break;

        case CharacterId::Reaper:
            // No human facial-muscle contract exists for Reaper. Expression is
            // skull/jaw/hood/spine language only.
            switch (event) {
                case CharacterPerformanceEvent::Idle:
                    frame.expression = CharacterExpression::ReaperStill;
                    frame.headRoll = -0.045f;
                    frame.cranialIntensity = 0.58f;
                    break;
                case CharacterPerformanceEvent::Locomotion:
                    frame.expression = CharacterExpression::ReaperRattle;
                    frame.headPitch = -0.06f;
                    frame.headRoll = 0.08f;
                    frame.torsoPitch = 0.10f;
                    frame.strideScale = 0.74f;
                    frame.staccatoWeight = 0.92f;
                    frame.cranialIntensity = 0.66f;
                    break;
                case CharacterPerformanceEvent::Dialogue:
                    frame.expression = CharacterExpression::ReaperOpenJaw;
                    frame.headRoll = 0.10f;
                    frame.jawOpen = 0.46f;
                    frame.cranialIntensity = 0.72f;
                    break;
                case CharacterPerformanceEvent::CombatReady:
                    frame.expression = CharacterExpression::ReaperRattle;
                    frame.headPitch = -0.14f;
                    frame.torsoPitch = 0.16f;
                    frame.leftArmBias = 0.22f;
                    frame.rightArmBias = -0.26f;
                    frame.snapWeight = 0.94f;
                    frame.staccatoWeight = 1.0f;
                    frame.cranialIntensity = 0.84f;
                    break;
                case CharacterPerformanceEvent::CombatImpact:
                    frame.expression = CharacterExpression::ReaperOpenJaw;
                    frame.headRoll = -0.18f;
                    frame.torsoRoll = 0.16f;
                    frame.jawOpen = 0.78f;
                    frame.snapWeight = 1.0f;
                    frame.staccatoWeight = 1.0f;
                    frame.cranialIntensity = 0.94f;
                    break;
                case CharacterPerformanceEvent::RealmShift:
                    frame.expression = CharacterExpression::ReaperRattle;
                    frame.headPitch = 0.12f;
                    frame.headRoll = 0.16f;
                    frame.jawOpen = 0.28f;
                    frame.snapWeight = 1.0f;
                    frame.staccatoWeight = 1.0f;
                    frame.cranialIntensity = 1.0f;
                    break;
                case CharacterPerformanceEvent::Overload:
                    frame.expression = CharacterExpression::ReaperOpenJaw;
                    frame.headPitch = 0.20f;
                    frame.headRoll = -0.22f;
                    frame.torsoPitch = -0.10f;
                    frame.torsoRoll = 0.18f;
                    frame.leftArmBias = -0.28f;
                    frame.rightArmBias = 0.30f;
                    frame.jawOpen = 1.0f;
                    frame.snapWeight = 1.0f;
                    frame.staccatoWeight = 1.0f;
                    frame.cranialIntensity = 1.0f;
                    break;
            }
            break;

        case CharacterId::None:
            break;
    }

    frame.headPitch = performanceClamp(frame.headPitch, -0.35f, 0.35f);
    frame.headRoll = performanceClamp(frame.headRoll, -0.35f, 0.35f);
    frame.torsoPitch = performanceClamp(frame.torsoPitch, -0.30f, 0.30f);
    frame.torsoRoll = performanceClamp(frame.torsoRoll, -0.30f, 0.30f);
    frame.leftArmBias = performanceClamp(frame.leftArmBias, -0.45f, 0.45f);
    frame.rightArmBias = performanceClamp(frame.rightArmBias, -0.45f, 0.45f);
    frame.strideScale = performanceClamp(frame.strideScale, 0.50f, 1.20f);
    frame.idleAmplitude = performanceClamp(frame.idleAmplitude, 0.0f, 1.0f);
    frame.gestureWeight = performanceClamp(frame.gestureWeight, 0.0f, 1.0f);
    frame.jawOpen = performanceClamp(frame.jawOpen, 0.0f, 1.0f);
    frame.cranialIntensity = performanceClamp(frame.cranialIntensity, 0.0f, 1.0f);
    frame.snapWeight = performanceClamp(frame.snapWeight, 0.0f, 1.0f);
    frame.staccatoWeight = performanceClamp(frame.staccatoWeight, 0.0f, 1.0f);
    return frame;
}

[[nodiscard]] constexpr CharacterPerformanceFrame composeCharacterPerformance(
    CharacterId characterId,
    MangaRenderEvent event
) noexcept
{
    return composeCharacterPerformance(characterId, performanceEventFor(event));
}

} // namespace hakui::character
