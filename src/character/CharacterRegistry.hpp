#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "character/CharacterIdentity.hpp"

namespace hakui::character {

using CharacterInstanceId = std::uint64_t;

inline constexpr CharacterInstanceId agnathosInstanceId = 0x4E53000000001001ULL;
inline constexpr CharacterInstanceId saelisInstanceId   = 0x4E53000000001002ULL;
inline constexpr CharacterInstanceId reaperInstanceId   = 0x4E53000000001003ULL;

enum class CharacterBindingKind : std::uint8_t {
    None,
    Player,
    Npc,
};

enum class CharacterLifecycleState : std::uint8_t {
    Despawned,
    Spawned,
};

struct CharacterBinding {
    CharacterBindingKind kind = CharacterBindingKind::None;
    std::uint32_t actorId = 0;
};

// L17 runtime instance record.
//
// This record owns character presence and the stable identity of that presence,
// but it does not own transform, health, navigation, animation or renderer
// state. Player/NPC authorities remain the source of physical simulation truth.
struct CharacterInstance {
    CharacterInstanceId instanceId = 0;
    CharacterId characterId = CharacterId::None;
    CharacterBinding binding{};
    CharacterLifecycleState lifecycle = CharacterLifecycleState::Despawned;
    std::uint32_t lifecycleRevision = 0;
};

class CharacterRegistry final {
public:
    explicit CharacterRegistry(std::uint32_t saelisNpcActorId = 0) noexcept
    {
        resetCanonicalRoster(saelisNpcActorId);
    }

    [[nodiscard]] std::span<CharacterInstance> instances() noexcept
    {
        return {instances_.data(), instances_.size()};
    }

    [[nodiscard]] std::span<const CharacterInstance> instances() const noexcept
    {
        return {instances_.data(), instances_.size()};
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return instances_.size();
    }

    [[nodiscard]] std::size_t spawnedCount() const noexcept
    {
        std::size_t count = 0;
        for (const CharacterInstance& instance : instances_) {
            if (instance.lifecycle == CharacterLifecycleState::Spawned) {
                ++count;
            }
        }
        return count;
    }

    [[nodiscard]] CharacterInstance* find(CharacterId characterId) noexcept
    {
        for (CharacterInstance& instance : instances_) {
            if (instance.characterId == characterId) return &instance;
        }
        return nullptr;
    }

    [[nodiscard]] const CharacterInstance* find(CharacterId characterId) const noexcept
    {
        for (const CharacterInstance& instance : instances_) {
            if (instance.characterId == characterId) return &instance;
        }
        return nullptr;
    }

    [[nodiscard]] CharacterInstance* find(CharacterInstanceId instanceId) noexcept
    {
        for (CharacterInstance& instance : instances_) {
            if (instance.instanceId == instanceId) return &instance;
        }
        return nullptr;
    }

    [[nodiscard]] const CharacterInstance* find(CharacterInstanceId instanceId) const noexcept
    {
        for (const CharacterInstance& instance : instances_) {
            if (instance.instanceId == instanceId) return &instance;
        }
        return nullptr;
    }

    [[nodiscard]] bool isSpawned(CharacterId characterId) const noexcept
    {
        const CharacterInstance* instance = find(characterId);
        return instance && instance->lifecycle == CharacterLifecycleState::Spawned;
    }

    bool spawn(CharacterId characterId) noexcept
    {
        CharacterInstance* instance = find(characterId);
        if (!instance || instance->lifecycle == CharacterLifecycleState::Spawned) {
            return false;
        }
        instance->lifecycle = CharacterLifecycleState::Spawned;
        ++instance->lifecycleRevision;
        return true;
    }

    bool despawn(CharacterId characterId) noexcept
    {
        CharacterInstance* instance = find(characterId);
        if (!instance || instance->lifecycle == CharacterLifecycleState::Despawned) {
            return false;
        }
        instance->lifecycle = CharacterLifecycleState::Despawned;
        ++instance->lifecycleRevision;
        return true;
    }

    // Restore the authored Neeshego roster using stable instance IDs. Session
    // reset may reconstruct simulation state, but canonical character instance
    // identity is deterministic and never depends on allocation order.
    void resetCanonicalRoster(std::uint32_t saelisNpcActorId) noexcept
    {
        instances_ = {{
            {
                agnathosInstanceId,
                CharacterId::Agnathos,
                {CharacterBindingKind::Player, 0},
                CharacterLifecycleState::Spawned,
                1,
            },
            {
                saelisInstanceId,
                CharacterId::Saelis,
                {CharacterBindingKind::Npc, saelisNpcActorId},
                CharacterLifecycleState::Spawned,
                1,
            },
            {
                reaperInstanceId,
                CharacterId::Reaper,
                {CharacterBindingKind::None, 0},
                CharacterLifecycleState::Spawned,
                1,
            },
        }};
    }

private:
    std::array<CharacterInstance, 3> instances_{};
};

} // namespace hakui::character
