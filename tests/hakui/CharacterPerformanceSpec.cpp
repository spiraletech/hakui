#include "character/CharacterPerformanceProfile.hpp"
#include "core/GameRuntime.hpp"

#include <cassert>

int main()
{
    using namespace hakui::character;

    const CharacterPerformanceProfile* agnathos =
        canonicalPerformanceProfile(CharacterId::Agnathos);
    const CharacterPerformanceProfile* saelis =
        canonicalPerformanceProfile(CharacterId::Saelis);
    const CharacterPerformanceProfile* reaper =
        canonicalPerformanceProfile(CharacterId::Reaper);

    assert(agnathos != nullptr);
    assert(saelis != nullptr);
    assert(reaper != nullptr);
    assert(canonicalPerformanceProfile(CharacterId::None) == nullptr);

    assert(agnathos->motion == CharacterMotionGrammar::GuardianStillness);
    assert(agnathos->defaultExpression == CharacterExpression::RasterBlank);
    assert(agnathos->usesHumanFacialMuscles);
    assert(agnathos->usesRasterEyeChannel);

    assert(saelis->motion == CharacterMotionGrammar::SyntheticPrecision);
    assert(saelis->defaultExpression == CharacterExpression::SyntheticNeutral);
    assert(saelis->usesHumanFacialMuscles);
    assert(!saelis->usesRasterEyeChannel);

    assert(reaper->motion == CharacterMotionGrammar::ReaperStaccato);
    assert(reaper->defaultExpression == CharacterExpression::ReaperStill);
    assert(!reaper->usesHumanFacialMuscles);
    assert(reaper->usesJawChannel);

    const CharacterPerformanceFrame agnathosDialogue =
        composeCharacterPerformance(CharacterId::Agnathos,
                                    CharacterPerformanceEvent::Dialogue);
    assert(agnathosDialogue.expression == CharacterExpression::RasterFocused);
    assert(agnathosDialogue.gestureWeight > agnathos->gestureAmplitude);
    assert(agnathosDialogue.cranialIntensity > 0.60f);

    const CharacterPerformanceFrame saelisDialogue =
        composeCharacterPerformance(CharacterId::Saelis,
                                    CharacterPerformanceEvent::Dialogue);
    assert(saelisDialogue.expression == CharacterExpression::SyntheticWarm);
    assert(saelisDialogue.snapWeight < 0.50f);

    const CharacterPerformanceFrame reaperDialogue =
        composeCharacterPerformance(CharacterId::Reaper,
                                    CharacterPerformanceEvent::Dialogue);
    assert(reaperDialogue.expression == CharacterExpression::ReaperOpenJaw);
    assert(reaperDialogue.jawOpen > 0.40f);

    const CharacterPerformanceFrame reaperOverload =
        composeCharacterPerformance(CharacterId::Reaper,
                                    CharacterPerformanceEvent::Overload);
    assert(reaperOverload.expression == CharacterExpression::ReaperOpenJaw);
    assert(reaperOverload.jawOpen == 1.0f);
    assert(reaperOverload.snapWeight == 1.0f);
    assert(reaperOverload.staccatoWeight == 1.0f);

    const CharacterPerformanceFrame agnathosRealm =
        composeCharacterPerformance(CharacterId::Agnathos,
                                    MangaRenderEvent::RealmShift);
    assert(agnathosRealm.expression == CharacterExpression::RasterFocused);
    assert(agnathosRealm.cranialIntensity == 1.0f);

    assert(performanceEventFor(MangaRenderEvent::Dialogue) ==
           CharacterPerformanceEvent::Dialogue);
    assert(performanceEventFor(MangaRenderEvent::CombatImpact) ==
           CharacterPerformanceEvent::CombatImpact);
    assert(performanceEventFor(MangaRenderEvent::Overload) ==
           CharacterPerformanceEvent::Overload);

    hakui::GameRuntime runtime;
    assert(runtime.playerPerformanceProfile() == agnathos);
    assert(runtime.npcPerformanceProfile(hakui::NpcManager::saelisId) == saelis);
    assert(runtime.characterPerformanceProfile(reaperInstanceId) == reaper);

    const CharacterPerformanceFrame playerDialogue =
        runtime.playerPerformanceFrame(CharacterPerformanceEvent::Dialogue);
    assert(playerDialogue.characterId == CharacterId::Agnathos);
    assert(playerDialogue.expression == CharacterExpression::RasterFocused);

    const CharacterPerformanceFrame runtimeReaper =
        runtime.characterPerformanceFrame(
            reaperInstanceId,
            CharacterPerformanceEvent::CombatReady
        );
    assert(runtimeReaper.characterId == CharacterId::Reaper);
    assert(runtimeReaper.motion == CharacterMotionGrammar::ReaperStaccato);
    assert(runtimeReaper.staccatoWeight == 1.0f);

    runtime.resetSession();
    assert(runtime.playerPerformanceProfile() == agnathos);
    assert(runtime.characterPerformanceProfile(reaperInstanceId) == reaper);

    return 0;
}
