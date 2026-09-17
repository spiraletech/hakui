#include "character/CharacterRegistry.hpp"
#include "core/GameRuntime.hpp"

#include <cassert>

int main()
{
    using namespace hakui::character;

    CharacterRegistry registry(hakui::NpcManager::saelisId);
    assert(registry.size() == 3);
    assert(registry.spawnedCount() == 3);

    const CharacterInstance* agnathos = registry.find(CharacterId::Agnathos);
    const CharacterInstance* saelis = registry.find(CharacterId::Saelis);
    const CharacterInstance* reaper = registry.find(CharacterId::Reaper);

    assert(agnathos != nullptr);
    assert(saelis != nullptr);
    assert(reaper != nullptr);

    assert(agnathos->instanceId == agnathosInstanceId);
    assert(saelis->instanceId == saelisInstanceId);
    assert(reaper->instanceId == reaperInstanceId);
    assert(agnathos->instanceId != saelis->instanceId);
    assert(agnathos->instanceId != reaper->instanceId);
    assert(saelis->instanceId != reaper->instanceId);

    assert(agnathos->binding.kind == CharacterBindingKind::Player);
    assert(agnathos->binding.actorId == 0);
    assert(saelis->binding.kind == CharacterBindingKind::Npc);
    assert(saelis->binding.actorId == hakui::NpcManager::saelisId);
    assert(reaper->binding.kind == CharacterBindingKind::None);
    assert(reaper->binding.actorId == 0);

    assert(registry.find(agnathosInstanceId) == agnathos);
    assert(registry.find(saelisInstanceId) == saelis);
    assert(registry.find(reaperInstanceId) == reaper);
    assert(registry.find(CharacterId::None) == nullptr);
    assert(registry.find(CharacterInstanceId{0}) == nullptr);

    assert(registry.isSpawned(CharacterId::Agnathos));
    assert(registry.isSpawned(CharacterId::Saelis));
    assert(registry.isSpawned(CharacterId::Reaper));

    const CharacterInstanceId originalReaperInstance = reaper->instanceId;
    const std::uint32_t originalRevision = reaper->lifecycleRevision;
    assert(registry.despawn(CharacterId::Reaper));
    assert(!registry.isSpawned(CharacterId::Reaper));
    assert(registry.spawnedCount() == 2);
    assert(!registry.despawn(CharacterId::Reaper));

    reaper = registry.find(CharacterId::Reaper);
    assert(reaper != nullptr);
    assert(reaper->instanceId == originalReaperInstance);
    assert(reaper->lifecycleRevision == originalRevision + 1);

    assert(registry.spawn(CharacterId::Reaper));
    assert(registry.isSpawned(CharacterId::Reaper));
    assert(registry.spawnedCount() == 3);
    assert(!registry.spawn(CharacterId::Reaper));
    reaper = registry.find(CharacterId::Reaper);
    assert(reaper->instanceId == originalReaperInstance);
    assert(reaper->lifecycleRevision == originalRevision + 2);
    assert(!registry.spawn(CharacterId::None));
    assert(!registry.despawn(CharacterId::None));

    // GameRuntime owns one registry beside existing player/NPC authorities.
    // Agnathos and Saelis are physically bound now; Reaper is a live roster
    // presence with no physical actor until the L18 embodiment layer.
    hakui::GameRuntime runtime;
    assert(runtime.characters().size() == 3);
    assert(runtime.characters().spawnedCount() == 3);

    const CharacterInstance* playerInstance = runtime.playerCharacterInstance();
    assert(playerInstance != nullptr);
    assert(playerInstance->characterId == CharacterId::Agnathos);
    assert(playerInstance->instanceId == agnathosInstanceId);
    assert(playerInstance->binding.kind == CharacterBindingKind::Player);

    const CharacterInstance* saelisInstance =
        runtime.npcCharacterInstance(hakui::NpcManager::saelisId);
    assert(saelisInstance != nullptr);
    assert(saelisInstance->characterId == CharacterId::Saelis);
    assert(saelisInstance->instanceId == saelisInstanceId);
    assert(saelisInstance->binding.kind == CharacterBindingKind::Npc);
    assert(saelisInstance->binding.actorId == hakui::NpcManager::saelisId);
    assert(runtime.npcCharacterInstance(999999) == nullptr);

    const CharacterInstance* runtimeReaper =
        runtime.characters().find(CharacterId::Reaper);
    assert(runtimeReaper != nullptr);
    assert(runtimeReaper->instanceId == reaperInstanceId);
    assert(runtimeReaper->binding.kind == CharacterBindingKind::None);
    assert(runtimeReaper->lifecycle == CharacterLifecycleState::Spawned);

    // A player movement reset must not mutate independent character lifecycle.
    assert(runtime.characters().despawn(CharacterId::Reaper));
    runtime.resetPlayerToSpawn();
    assert(!runtime.characters().isSpawned(CharacterId::Reaper));
    assert(runtime.playerCharacterInstance()->instanceId == agnathosInstanceId);

    // Full session reset restores authored presence but preserves canonical
    // instance identity, so references never depend on allocator order.
    runtime.resetSession();
    assert(runtime.characters().spawnedCount() == 3);
    assert(runtime.characters().find(CharacterId::Agnathos)->instanceId ==
           agnathosInstanceId);
    assert(runtime.characters().find(CharacterId::Saelis)->instanceId ==
           saelisInstanceId);
    assert(runtime.characters().find(CharacterId::Reaper)->instanceId ==
           reaperInstanceId);

    return 0;
}
