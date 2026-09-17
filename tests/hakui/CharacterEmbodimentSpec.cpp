#include "character/CharacterEmbodiment.hpp"
#include "character/CharacterRenderProfile.hpp"
#include "core/GameRuntime.hpp"

#include <cassert>

int main()
{
    using namespace hakui::character;

    const CharacterEmbodimentProfile* agnathos =
        canonicalEmbodiment(CharacterId::Agnathos);
    const CharacterEmbodimentProfile* saelis =
        canonicalEmbodiment(CharacterId::Saelis);
    const CharacterEmbodimentProfile* reaper =
        canonicalEmbodiment(CharacterId::Reaper);

    assert(agnathos != nullptr);
    assert(saelis != nullptr);
    assert(reaper != nullptr);
    assert(canonicalEmbodiment(CharacterId::None) == nullptr);

    assert(agnathos->rig == CharacterRigArchetype::Humanoid);
    assert(agnathos->bodyProfile.has_value());
    assert(*agnathos->bodyProfile == hakui::avatar::BodyProfileId::Male);
    assert(agnathos->visualState == CharacterVisualState::RasterVeil);
    assert(agnathos->surface == CharacterSurfaceLanguage::RasterVeil);
    assert(agnathos->hasHumanFace);
    assert(agnathos->hasHair);

    assert(saelis->rig == CharacterRigArchetype::Humanoid);
    assert(saelis->bodyProfile.has_value());
    assert(*saelis->bodyProfile == hakui::avatar::BodyProfileId::Female);
    assert(saelis->surface == CharacterSurfaceLanguage::SaelisSynthetic);

    assert(reaper->rig == CharacterRigArchetype::ReaperSkeleton);
    assert(!reaper->bodyProfile.has_value());
    assert(reaper->visualState == CharacterVisualState::ReaperSkeleton);
    assert(reaper->surface == CharacterSurfaceLanguage::ReaperXerox);
    assert(!reaper->hasHumanFace);
    assert(!reaper->hasHair);

    const CharacterRenderProfile* agnathosRender =
        canonicalRenderProfile(CharacterId::Agnathos);
    const CharacterRenderProfile* saelisRender =
        canonicalRenderProfile(CharacterId::Saelis);
    const CharacterRenderProfile* reaperRender =
        canonicalRenderProfile(CharacterId::Reaper);

    assert(agnathosRender != nullptr);
    assert(saelisRender != nullptr);
    assert(reaperRender != nullptr);
    assert(canonicalRenderProfile(CharacterId::None) == nullptr);

    assert(agnathosRender->monochromeOnly);
    assert(agnathosRender->whiteSubjectOnBlack);
    assert(agnathosRender->inkMode == MangaInkMode::RasterInk);
    assert(agnathosRender->scanlines > agnathosRender->xeroxNoise);
    assert(agnathosRender->allowFrameTear);

    assert(saelisRender->monochromeOnly);
    assert(saelisRender->inkMode == MangaInkMode::SyntheticInk);
    assert(saelisRender->halftone > saelisRender->scanlines);
    assert(!saelisRender->allowFrameTear);

    assert(reaperRender->monochromeOnly);
    assert(reaperRender->whiteSubjectOnBlack);
    assert(reaperRender->inkMode == MangaInkMode::XeroxCrush);
    assert(reaperRender->xeroxNoise > reaperRender->scanlines);
    assert(reaperRender->edgeInk == 1.0f);

    const MangaFrameTreatment agnathosExplore = composeMangaFrame(
        CharacterId::Agnathos,
        MangaRenderEvent::Exploration
    );
    assert(agnathosExplore.characterId == CharacterId::Agnathos);
    assert(agnathosExplore.panel == MangaPanelMode::World);
    assert(agnathosExplore.monochrome);
    assert(agnathosExplore.whiteSubjectOnBlack);
    assert(agnathosExplore.scanlines == agnathosRender->scanlines);

    const MangaFrameTreatment agnathosImpact = composeMangaFrame(
        CharacterId::Agnathos,
        MangaRenderEvent::CombatImpact
    );
    assert(agnathosImpact.panel == MangaPanelMode::ImpactPanel);
    assert(agnathosImpact.speedLines == 1.0f);
    assert(agnathosImpact.blackField == 1.0f);
    assert(agnathosImpact.freezeFrame > 0.8f);
    assert(agnathosImpact.frameTear);

    const MangaFrameTreatment agnathosShift = composeMangaFrame(
        CharacterId::Agnathos,
        MangaRenderEvent::RealmShift
    );
    assert(agnathosShift.panel == MangaPanelMode::RealmBreak);
    assert(agnathosShift.frameTear);
    assert(agnathosShift.scanlines > agnathosExplore.scanlines);

    const MangaFrameTreatment saelisDialogue = composeMangaFrame(
        CharacterId::Saelis,
        MangaRenderEvent::Dialogue
    );
    assert(saelisDialogue.panel == MangaPanelMode::PortraitCut);
    assert(!saelisDialogue.frameTear);

    const MangaFrameTreatment reaperOverload = composeMangaFrame(
        CharacterId::Reaper,
        MangaRenderEvent::Overload
    );
    assert(reaperOverload.panel == MangaPanelMode::RealmBreak);
    assert(reaperOverload.xeroxNoise == 1.0f);
    assert(reaperOverload.blackField == 1.0f);
    assert(reaperOverload.frameTear);
    assert(reaperOverload.edgeInk == 1.0f);

    const MangaFrameTreatment unknown = composeMangaFrame(
        CharacterId::None,
        MangaRenderEvent::CombatImpact
    );
    assert(unknown.characterId == CharacterId::None);
    assert(unknown.speedLines == 0.0f);

    hakui::GameRuntime runtime;
    assert(runtime.playerEmbodiment() == agnathos);
    assert(runtime.npcEmbodiment(hakui::NpcManager::saelisId) == saelis);
    assert(runtime.npcEmbodiment(999999) == nullptr);
    assert(runtime.characterEmbodiment(agnathosInstanceId) == agnathos);
    assert(runtime.characterEmbodiment(saelisInstanceId) == saelis);
    assert(runtime.characterEmbodiment(reaperInstanceId) == reaper);
    assert(runtime.characterEmbodiment(CharacterInstanceId{0}) == nullptr);

    assert(runtime.playerRenderProfile() == agnathosRender);
    assert(runtime.npcRenderProfile(hakui::NpcManager::saelisId) == saelisRender);
    assert(runtime.npcRenderProfile(999999) == nullptr);
    assert(runtime.characterRenderProfile(agnathosInstanceId) == agnathosRender);
    assert(runtime.characterRenderProfile(saelisInstanceId) == saelisRender);
    assert(runtime.characterRenderProfile(reaperInstanceId) == reaperRender);
    assert(runtime.characterRenderProfile(CharacterInstanceId{0}) == nullptr);

    const MangaFrameTreatment runtimeImpact = runtime.playerMangaFrame(
        MangaRenderEvent::CombatImpact
    );
    assert(runtimeImpact.characterId == CharacterId::Agnathos);
    assert(runtimeImpact.panel == MangaPanelMode::ImpactPanel);

    const MangaFrameTreatment runtimeReaperShift = runtime.characterMangaFrame(
        reaperInstanceId,
        MangaRenderEvent::RealmShift
    );
    assert(runtimeReaperShift.characterId == CharacterId::Reaper);
    assert(runtimeReaperShift.panel == MangaPanelMode::RealmBreak);
    assert(runtimeReaperShift.xeroxNoise > reaperRender->xeroxNoise);

    runtime.resetSession();
    assert(runtime.playerEmbodiment() == agnathos);
    assert(runtime.characterEmbodiment(reaperInstanceId) == reaper);
    assert(runtime.playerRenderProfile() == agnathosRender);
    assert(runtime.characterRenderProfile(reaperInstanceId) == reaperRender);

    return 0;
}
