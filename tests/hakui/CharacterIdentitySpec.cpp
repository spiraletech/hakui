#include "character/CharacterIdentity.hpp"
#include "core/GameRuntime.hpp"

#include <cassert>

int main()
{
    using namespace hakui::character;

    const CharacterIdentity* agnathos = canonicalIdentity(CharacterId::Agnathos);
    assert(agnathos != nullptr);
    assert(agnathos == canonicalIdentity("agnathos"));
    assert(agnathos->canonicalName == "AGNATHOS");
    assert(agnathos->species == CharacterSpecies::Human);
    assert(agnathos->faction == CharacterFaction::SpiralGuardian);
    assert(agnathos->realmAlignment == RealmAlignment::SpiralGrove);
    assert(agnathos->baseBodyProfile.has_value());
    assert(*agnathos->baseBodyProfile == hakui::avatar::BodyProfileId::Male);
    assert(agnathos->faceIdentity == "agnathos.base-face.v1");
    assert(agnathos->defaultVisualState == CharacterVisualState::RasterVeil);
    assert(agnathos->embodiment == EmbodimentClass::Humanoid);
    assert(agnathos->playable);

    const CharacterIdentity* saelis = canonicalIdentity(CharacterId::Saelis);
    assert(saelis != nullptr);
    assert(saelis == canonicalIdentity("saelis"));
    assert(saelis->canonicalName == "SAELIS");
    assert(saelis->baseBodyProfile.has_value());
    assert(*saelis->baseBodyProfile == hakui::avatar::BodyProfileId::Female);
    assert(saelis->defaultVisualState == CharacterVisualState::SaelisDefault);

    const CharacterIdentity* reaper = canonicalIdentity(CharacterId::Reaper);
    assert(reaper != nullptr);
    assert(reaper == canonicalIdentity("reaper"));
    assert(reaper->canonicalName == "THE REAPER");
    assert(reaper->species == CharacterSpecies::ReaperSkeleton);
    assert(!reaper->baseBodyProfile.has_value());
    assert(reaper->faceIdentity == "none.skeletal");
    assert(reaper->defaultVisualState == CharacterVisualState::ReaperSkeleton);
    assert(reaper->embodiment == EmbodimentClass::NonHumanSkeleton);
    assert(!reaper->playable);

    assert(canonicalIdentity(CharacterId::None) == nullptr);
    assert(canonicalIdentity("unknown") == nullptr);

    hakui::GameRuntime runtime;
    assert(runtime.player().characterId == CharacterId::Agnathos);
    assert(runtime.playerIdentity() == agnathos);

    const hakui::NpcState* saelisState =
        runtime.npcs().find(hakui::NpcManager::saelisId);
    assert(saelisState != nullptr);
    assert(saelisState->characterId == CharacterId::Saelis);
    assert(runtime.npcIdentity(hakui::NpcManager::saelisId) == saelis);
    assert(runtime.npcIdentity(999999) == nullptr);

    // Reset paths may reconstruct player/NPC state, but canonical identity must
    // survive by deterministic rebinding rather than mutable prose or AI state.
    runtime.player().characterId = CharacterId::Reaper;
    runtime.resetPlayerToSpawn();
    assert(runtime.player().characterId == CharacterId::Agnathos);
    assert(runtime.playerIdentity() == agnathos);

    hakui::NpcState* mutableSaelis =
        runtime.npcs().find(hakui::NpcManager::saelisId);
    assert(mutableSaelis != nullptr);
    mutableSaelis->characterId = CharacterId::None;
    runtime.resetSession();
    saelisState = runtime.npcs().find(hakui::NpcManager::saelisId);
    assert(saelisState != nullptr);
    assert(saelisState->characterId == CharacterId::Saelis);
    assert(runtime.npcIdentity(hakui::NpcManager::saelisId) == saelis);

    return 0;
}
