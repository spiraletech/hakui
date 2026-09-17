#include "character/CharacterEmbodiment.hpp"
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

    hakui::GameRuntime runtime;
    assert(runtime.playerEmbodiment() == agnathos);
    assert(runtime.npcEmbodiment(hakui::NpcManager::saelisId) == saelis);
    assert(runtime.npcEmbodiment(999999) == nullptr);
    assert(runtime.characterEmbodiment(agnathosInstanceId) == agnathos);
    assert(runtime.characterEmbodiment(saelisInstanceId) == saelis);
    assert(runtime.characterEmbodiment(reaperInstanceId) == reaper);
    assert(runtime.characterEmbodiment(CharacterInstanceId{0}) == nullptr);

    runtime.resetSession();
    assert(runtime.playerEmbodiment() == agnathos);
    assert(runtime.characterEmbodiment(reaperInstanceId) == reaper);

    return 0;
}
