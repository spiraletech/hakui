#pragma once

#include "action/HakuiActionGate.hpp"
#include "character/CharacterActorAuthority.hpp"
#include "character/CharacterEmbodiment.hpp"
#include "character/CharacterIdentity.hpp"
#include "character/CharacterPerformanceProfile.hpp"
#include "character/CharacterPoseExecutor.hpp"
#include "character/CharacterRegistry.hpp"
#include "character/CharacterRenderProfile.hpp"
#include "interaction/InteractionRegistry.hpp"
#include "npc/NpcManager.hpp"
#include "player/PlayerRuntime.hpp"
#include "witness/HakuiWitness.hpp"
#include "world/HakuiWorldState.hpp"

namespace hakui {

// Deterministic authority root.
//
// L10 extends the L5 split with an explicit NPC authority. World, player,
// residents and interaction membership each have one owner while platform
// input, rendering, audio, chat, combat and Spiral orchestration remain outside
// this class. L16 adds immutable character identity references. L17 adds a
// fixed-budget character registry for stable instance identity and lifecycle.
// L18 adds immutable embodiment profiles. L19 adds immutable manga render
// grammar plus deterministic frame-treatment composition without moving GPU or
// presentation-resource ownership into gameplay state. L21 adds immutable
// character performance canon. L22 resolves those authored samples into
// rig-space pose channels without taking root-motion or transform authority.
// L23 adds independent character-actor authority for spawned canonical cast
// members that are intentionally not PlayerRuntime or NpcManager entities.
class GameRuntime final {
public:
    GameRuntime() noexcept
        : characters_(NpcManager::saelisId)
    {
        bindCanonicalCharacters();
        resetIndependentCharacterActors();
    }
    GameRuntime(const GameRuntime&) = delete;
    GameRuntime& operator=(const GameRuntime&) = delete;
    GameRuntime(GameRuntime&&) = delete;
    GameRuntime& operator=(GameRuntime&&) = delete;

    HakuiWorldState& world() noexcept { return world_; }
    const HakuiWorldState& world() const noexcept { return world_; }

    PlayerRuntime& playerRuntime() noexcept { return player_; }
    const PlayerRuntime& playerRuntime() const noexcept { return player_; }

    NpcManager& npcs() noexcept { return npcs_; }
    const NpcManager& npcs() const noexcept { return npcs_; }

    character::CharacterRegistry& characters() noexcept { return characters_; }
    const character::CharacterRegistry& characters() const noexcept { return characters_; }

    character::CharacterActorAuthority& characterActors() noexcept
    {
        return characterActors_;
    }

    const character::CharacterActorAuthority& characterActors() const noexcept
    {
        return characterActors_;
    }

    HakuiActionGate& actionGate() noexcept { return actionGate_; }
    const HakuiActionGate& actionGate() const noexcept { return actionGate_; }

    witness::HakuiWitness& witness() noexcept { return witness_; }
    const witness::HakuiWitness& witness() const noexcept { return witness_; }

    InteractionRegistry& interactionRegistry() noexcept { return interactions_; }
    const InteractionRegistry& interactionRegistry() const noexcept { return interactions_; }

    [[nodiscard]] const character::CharacterIdentity* playerIdentity() const noexcept
    {
        return character::canonicalIdentity(player_.state().characterId);
    }

    [[nodiscard]] const character::CharacterEmbodimentProfile* playerEmbodiment() const noexcept
    {
        return character::canonicalEmbodiment(player_.state().characterId);
    }

    [[nodiscard]] const character::CharacterRenderProfile* playerRenderProfile() const noexcept
    {
        return character::canonicalRenderProfile(player_.state().characterId);
    }

    [[nodiscard]] const character::CharacterPerformanceProfile* playerPerformanceProfile() const noexcept
    {
        return character::canonicalPerformanceProfile(player_.state().characterId);
    }

    [[nodiscard]] character::MangaFrameTreatment playerMangaFrame(
        character::MangaRenderEvent event
    ) const noexcept
    {
        return character::composeMangaFrame(player_.state().characterId, event);
    }

    [[nodiscard]] character::CharacterPerformanceFrame playerPerformanceFrame(
        character::CharacterPerformanceEvent event
    ) const noexcept
    {
        return character::composeCharacterPerformance(player_.state().characterId, event);
    }

    [[nodiscard]] character::CharacterPerformanceFrame playerPerformanceFrame(
        character::MangaRenderEvent event
    ) const noexcept
    {
        return character::composeCharacterPerformance(player_.state().characterId, event);
    }

    [[nodiscard]] character::CharacterPoseExecution playerPoseExecution(
        character::CharacterPerformanceEvent event,
        float phase01
    ) const noexcept
    {
        return character::executeCharacterPerformance(
            player_.state().characterId,
            event,
            phase01
        );
    }

    [[nodiscard]] character::CharacterPoseExecution playerPoseExecution(
        character::MangaRenderEvent event,
        float phase01
    ) const noexcept
    {
        return character::executeCharacterPerformance(
            player_.state().characterId,
            event,
            phase01
        );
    }

    [[nodiscard]] const character::CharacterIdentity* npcIdentity(
        std::uint32_t npcId
    ) const noexcept
    {
        const NpcState* npc = npcs_.find(npcId);
        return npc ? character::canonicalIdentity(npc->characterId) : nullptr;
    }

    [[nodiscard]] const character::CharacterEmbodimentProfile* npcEmbodiment(
        std::uint32_t npcId
    ) const noexcept
    {
        const NpcState* npc = npcs_.find(npcId);
        return npc ? character::canonicalEmbodiment(npc->characterId) : nullptr;
    }

    [[nodiscard]] const character::CharacterRenderProfile* npcRenderProfile(
        std::uint32_t npcId
    ) const noexcept
    {
        const NpcState* npc = npcs_.find(npcId);
        return npc ? character::canonicalRenderProfile(npc->characterId) : nullptr;
    }

    [[nodiscard]] const character::CharacterPerformanceProfile* npcPerformanceProfile(
        std::uint32_t npcId
    ) const noexcept
    {
        const NpcState* npc = npcs_.find(npcId);
        return npc ? character::canonicalPerformanceProfile(npc->characterId) : nullptr;
    }

    [[nodiscard]] character::CharacterPoseExecution npcPoseExecution(
        std::uint32_t npcId,
        character::CharacterPerformanceEvent event,
        float phase01
    ) const noexcept
    {
        const NpcState* npc = npcs_.find(npcId);
        return character::executeCharacterPerformance(
            npc ? npc->characterId : character::CharacterId::None,
            event,
            phase01
        );
    }

    [[nodiscard]] const character::CharacterEmbodimentProfile* characterEmbodiment(
        character::CharacterInstanceId instanceId
    ) const noexcept
    {
        const character::CharacterInstance* instance = characters_.find(instanceId);
        return instance ? character::canonicalEmbodiment(instance->characterId) : nullptr;
    }

    [[nodiscard]] const character::CharacterRenderProfile* characterRenderProfile(
        character::CharacterInstanceId instanceId
    ) const noexcept
    {
        const character::CharacterInstance* instance = characters_.find(instanceId);
        return instance ? character::canonicalRenderProfile(instance->characterId) : nullptr;
    }

    [[nodiscard]] const character::CharacterPerformanceProfile* characterPerformanceProfile(
        character::CharacterInstanceId instanceId
    ) const noexcept
    {
        const character::CharacterInstance* instance = characters_.find(instanceId);
        return instance ? character::canonicalPerformanceProfile(instance->characterId) : nullptr;
    }

    [[nodiscard]] character::MangaFrameTreatment characterMangaFrame(
        character::CharacterInstanceId instanceId,
        character::MangaRenderEvent event
    ) const noexcept
    {
        const character::CharacterInstance* instance = characters_.find(instanceId);
        return character::composeMangaFrame(
            instance ? instance->characterId : character::CharacterId::None,
            event
        );
    }

    [[nodiscard]] character::CharacterPerformanceFrame characterPerformanceFrame(
        character::CharacterInstanceId instanceId,
        character::CharacterPerformanceEvent event
    ) const noexcept
    {
        const character::CharacterInstance* instance = characters_.find(instanceId);
        return character::composeCharacterPerformance(
            instance ? instance->characterId : character::CharacterId::None,
            event
        );
    }

    [[nodiscard]] character::CharacterPoseExecution characterPoseExecution(
        character::CharacterInstanceId instanceId,
        character::CharacterPerformanceEvent event,
        float phase01
    ) const noexcept
    {
        const character::CharacterInstance* instance = characters_.find(instanceId);
        return character::executeCharacterPerformance(
            instance ? instance->characterId : character::CharacterId::None,
            event,
            phase01
        );
    }

    [[nodiscard]] character::CharacterInstance* playerCharacterInstance() noexcept
    {
        return characters_.find(player_.state().characterId);
    }

    [[nodiscard]] const character::CharacterInstance* playerCharacterInstance() const noexcept
    {
        return characters_.find(player_.state().characterId);
    }

    [[nodiscard]] character::CharacterInstance* npcCharacterInstance(
        std::uint32_t npcId
    ) noexcept
    {
        NpcState* npc = npcs_.find(npcId);
        return npc ? characters_.find(npc->characterId) : nullptr;
    }

    [[nodiscard]] const character::CharacterInstance* npcCharacterInstance(
        std::uint32_t npcId
    ) const noexcept
    {
        const NpcState* npc = npcs_.find(npcId);
        return npc ? characters_.find(npc->characterId) : nullptr;
    }

    [[nodiscard]] character::CharacterActorState* independentCharacterActor(
        character::CharacterId characterId
    ) noexcept
    {
        return characterActors_.find(characterId);
    }

    [[nodiscard]] const character::CharacterActorState* independentCharacterActor(
        character::CharacterId characterId
    ) const noexcept
    {
        return characterActors_.find(characterId);
    }

    [[nodiscard]] const character::CharacterActorState* independentCharacterActor(
        character::CharacterInstanceId instanceId
    ) const noexcept
    {
        return characterActors_.find(instanceId);
    }

    bool requestIndependentCharacterWalkTo(
        character::CharacterId characterId,
        float x,
        float y,
        float z
    ) noexcept
    {
        return characterActors_.requestWalkTo(characterId, x, y, z);
    }

    [[nodiscard]] bool independentCharacterInInteractionRange(
        character::CharacterId characterId
    ) const noexcept
    {
        return characterActors_.canInteract(characterId);
    }

    BlackRoom& blackRoom() noexcept { return world_.blackRoom(); }
    const BlackRoom& blackRoom() const noexcept { return world_.blackRoom(); }

    PlayerState& player() noexcept { return player_.state(); }
    const PlayerState& player() const noexcept { return player_.state(); }

    PlayerMovementController& movement() noexcept { return player_.movement(); }
    const PlayerMovementController& movement() const noexcept { return player_.movement(); }

    RideableMovementController& rideable() noexcept { return player_.rideable(); }
    const RideableMovementController& rideable() const noexcept { return player_.rideable(); }

    void advanceWorld(float deltaSeconds) noexcept
    {
        const std::uint64_t beforeStep = world_.clock().step();
        world_.advance(deltaSeconds);
        if (world_.clock().step() != beforeStep) {
            npcs_.tick(world_.blackRoom(), player_.state(), deltaSeconds);
            characterActors_.tick(
                player_.state().x,
                player_.state().y,
                player_.state().z,
                deltaSeconds
            );
        }
    }

    void resetPlayerToSpawn(float startingMoney = 250.0f) noexcept
    {
        player_.resetToSpawn(world_.blackRoom().movementEnvironment(), startingMoney);
        player_.state().characterId = character::CharacterId::Agnathos;
    }

    void resetSession(float startingMoney = 250.0f) noexcept
    {
        world_.reset();
        resetPlayerToSpawn(startingMoney);
        npcs_.reset(world_.blackRoom());
        bindCanonicalCharacters();
        characters_.resetCanonicalRoster(NpcManager::saelisId);
        resetIndependentCharacterActors();
        witness_.observed(
            world_.clock().step(),
            world_.elapsedSeconds,
            witness::WitnessKind::Mutation,
            "runtime.session",
            "authoritative world, player, NPC, character roster, independent actor, embodiment, render, performance, and pose canon reset"
        );
    }

private:
    void bindCanonicalCharacters() noexcept
    {
        player_.state().characterId = character::CharacterId::Agnathos;
        if (NpcState* saelis = npcs_.find(NpcManager::saelisId)) {
            saelis->characterId = character::CharacterId::Saelis;
        }
    }

    void resetIndependentCharacterActors() noexcept
    {
        const MovementEnvironment environment =
            world_.blackRoom().movementEnvironment();
        characterActors_.reset(
            environment.spawnX,
            environment.spawnY,
            environment.spawnZ,
            BlackRoom::floorMinimumX,
            BlackRoom::floorMaximumX,
            BlackRoom::floorMinimumZ,
            BlackRoom::floorMaximumZ
        );
    }

    HakuiWorldState world_{};
    PlayerRuntime player_{};
    NpcManager npcs_{};
    character::CharacterRegistry characters_;
    character::CharacterActorAuthority characterActors_{};
    HakuiActionGate actionGate_{};
    witness::HakuiWitness witness_{256};
    InteractionRegistry interactions_{};
};

} // namespace hakui
