#include "character/NeeshegoRenderExecution.hpp"

#include <cassert>

int main()
{
    using namespace hakui::character;

    const NeeshegoRenderExecution agnathos = executeMangaEvent(
        CharacterId::Agnathos,
        MangaRenderEvent::Exploration
    );
    assert(agnathos.active);
    assert(agnathos.characterId == CharacterId::Agnathos);
    assert(agnathos.inkMode == MangaInkMode::RasterInk);
    assert(agnathos.palette == MangaPaletteExecution::RasterMonochrome);
    assert(agnathos.monochrome);
    assert(agnathos.whiteSubjectOnBlack);
    assert(agnathos.scanlineCount >= 12);
    assert(agnathos.scanlineAlpha > 0.0f);

    const NeeshegoRenderExecution saelis = executeMangaEvent(
        CharacterId::Saelis,
        MangaRenderEvent::Exploration
    );
    assert(saelis.active);
    assert(saelis.inkMode == MangaInkMode::SyntheticInk);
    assert(saelis.palette == MangaPaletteExecution::SyntheticMonochrome);
    assert(saelis.xeroxStreakCount < agnathos.xeroxStreakCount);
    assert(!saelis.frameTear);

    const NeeshegoRenderExecution reaper = executeMangaEvent(
        CharacterId::Reaper,
        MangaRenderEvent::Exploration
    );
    assert(reaper.active);
    assert(reaper.inkMode == MangaInkMode::XeroxCrush);
    assert(reaper.palette == MangaPaletteExecution::XeroxMonochrome);
    assert(reaper.xeroxStreakCount > agnathos.xeroxStreakCount);
    assert(reaper.xeroxAlpha > agnathos.xeroxAlpha);

    const NeeshegoRenderExecution impact = executeMangaEvent(
        CharacterId::Agnathos,
        MangaRenderEvent::CombatImpact
    );
    assert(impact.panel == MangaPanelMode::ImpactPanel);
    assert(impact.panelBorder);
    assert(impact.speedLineCount > 0);
    assert(impact.impactFlashAlpha > 0.0f);
    assert(impact.frameTear);
    assert(impact.tearBandCount > 0);

    const NeeshegoRenderExecution dialogue = executeMangaEvent(
        CharacterId::Agnathos,
        MangaRenderEvent::Dialogue
    );
    assert(dialogue.panel == MangaPanelMode::PortraitCut);
    assert(dialogue.panelBars);
    assert(!dialogue.panelBorder);

    const NeeshegoRenderExecution realm = executeMangaEvent(
        CharacterId::Reaper,
        MangaRenderEvent::RealmShift
    );
    assert(realm.panel == MangaPanelMode::RealmBreak);
    assert(realm.panelBorder);
    assert(realm.frameTear);
    assert(realm.tearBandCount > 0);

    const NeeshegoRenderExecution none = executeMangaEvent(
        CharacterId::None,
        MangaRenderEvent::Exploration
    );
    assert(!none.active);
    assert(none.characterId == CharacterId::None);
    assert(none.palette == MangaPaletteExecution::Preserve);

    return 0;
}
