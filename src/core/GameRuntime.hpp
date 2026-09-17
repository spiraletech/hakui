#pragma once

#include "action/HakuiActionGate.hpp"
#include "character/CharacterEmbodiment.hpp"
#include "character/CharacterIdentity.hpp"
#include "character/CharacterRegistry.hpp"
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
// L18 adds immutable embodiment profiles without moving physical simulation
// ownership out of player/NPC authorities.
class GameRuntime final {
public:
    GameRuntime() noexcept
        : characters_(NpcManager::saelisId)
    {
        bindCanonicalCharacters();
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

    [[nodiscard]] const character::CharacterEmbodimentProfile* characterEmbodiment(
        character::CharacterInstanceId instanceId
    ) const noexcept
    {
        const character::CharacterInstance* instance = characters_.find(instanceId);
        return instance ? character::canonicalEmbodiment(instance->characterId) : nullptr;
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
        witness_.observed(
            world_.clock().step(),
            world_.elapsedSeconds,
            witness::WitnessKind::Mutation,
            "runtime.session",
            "authoritative world, player, NPC, character roster, and embodiment bindings reset"
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

    HakuiWorldState world_{};
    PlayerRuntime player_{};
    NpcManager npcs_{};
    character::CharacterRegistry characters_;
    HakuiActionGate actionGate_{};
    witness::HakuiWitness witness_{256};
    InteractionRegistry interactions_{};
};

} // namespace hakui
