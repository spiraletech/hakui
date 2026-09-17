#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <span>

#include "character/CharacterRegistry.hpp"

namespace hakui::character {

enum class CharacterActorActivity : std::uint8_t {
    Idle,
    Walking,
    ObservingPlayer,
};

// Authoritative simulation state for canonical characters that are not bound
// to PlayerRuntime or NpcManager. Bound characters keep their existing owners;
// this authority exists specifically so characters such as The Reaper can have
// a real world transform without being forced through a humanoid NPC path.
struct CharacterActorState {
    CharacterInstanceId instanceId = 0;
    CharacterId characterId = CharacterId::None;
    bool active = false;
    bool grounded = true;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float yaw = 0.0f;

    float spawnX = 0.0f;
    float spawnY = 0.0f;
    float spawnZ = 0.0f;
    float spawnYaw = 0.0f;

    float targetX = 0.0f;
    float targetY = 0.0f;
    float targetZ = 0.0f;
    float velocityX = 0.0f;
    float velocityZ = 0.0f;

    float movementBlend = 0.0f;
    float gaitPhase = 0.0f;
    float idlePhase = 0.0f;

    CharacterActorActivity activity = CharacterActorActivity::Idle;
    float interactionRadius = 2.25f;
    float observationRadius = 6.50f;
    float playerDistance = 100000.0f;
    bool playerInInteractionRange = false;
    std::uint64_t simulationTicks = 0;
};

// Native renderer mirror. This is a copy-only presentation seam, never the
// simulation authority. GameRuntime publishes after authoritative mutation;
// renderer code can inspect this view without owning or mutating actor state.
struct CharacterActorPresentationMirror {
    std::array<CharacterActorState, 8> actors{};
    std::size_t count = 0;
};

inline CharacterActorPresentationMirror& mutableCharacterActorPresentationMirror() noexcept
{
    static CharacterActorPresentationMirror mirror{};
    return mirror;
}

[[nodiscard]] inline std::span<const CharacterActorState>
publishedCharacterActors() noexcept
{
    const CharacterActorPresentationMirror& mirror =
        mutableCharacterActorPresentationMirror();
    return {mirror.actors.data(), mirror.count};
}

class CharacterActorAuthority final {
public:
    static constexpr std::size_t capacity = 8;
    static constexpr float reaperWalkSpeed = 1.55f;

    CharacterActorAuthority() noexcept
    {
        reset(0.0f, 0.0f, 0.0f, -10.0f, 10.0f, -8.0f, 8.0f);
    }

    [[nodiscard]] std::span<CharacterActorState> states() noexcept
    {
        return {actors_.data(), count_};
    }

    [[nodiscard]] std::span<const CharacterActorState> states() const noexcept
    {
        return {actors_.data(), count_};
    }

    [[nodiscard]] std::size_t size() const noexcept { return count_; }

    [[nodiscard]] CharacterActorState* find(CharacterId id) noexcept
    {
        for (std::size_t index = 0; index < count_; ++index) {
            if (actors_[index].characterId == id) return &actors_[index];
        }
        return nullptr;
    }

    [[nodiscard]] const CharacterActorState* find(CharacterId id) const noexcept
    {
        for (std::size_t index = 0; index < count_; ++index) {
            if (actors_[index].characterId == id) return &actors_[index];
        }
        return nullptr;
    }

    [[nodiscard]] CharacterActorState* find(CharacterInstanceId id) noexcept
    {
        for (std::size_t index = 0; index < count_; ++index) {
            if (actors_[index].instanceId == id) return &actors_[index];
        }
        return nullptr;
    }

    [[nodiscard]] const CharacterActorState* find(CharacterInstanceId id) const noexcept
    {
        for (std::size_t index = 0; index < count_; ++index) {
            if (actors_[index].instanceId == id) return &actors_[index];
        }
        return nullptr;
    }

    [[nodiscard]] bool canInteract(CharacterId id) const noexcept
    {
        const CharacterActorState* actor = find(id);
        return actor && actor->active && actor->playerInInteractionRange;
    }

    bool requestWalkTo(CharacterId id, float x, float y, float z) noexcept
    {
        CharacterActorState* actor = find(id);
        if (!actor || !actor->active) return false;
        actor->targetX = std::clamp(x, minimumX_, maximumX_);
        actor->targetY = y;
        actor->targetZ = std::clamp(z, minimumZ_, maximumZ_);
        actor->activity = CharacterActorActivity::Walking;
        return true;
    }

    bool requestReturnToSpawn(CharacterId id) noexcept
    {
        CharacterActorState* actor = find(id);
        return actor && requestWalkTo(
            id, actor->spawnX, actor->spawnY, actor->spawnZ
        );
    }

    void reset(
        float worldSpawnX,
        float worldSpawnY,
        float worldSpawnZ,
        float minimumX,
        float maximumX,
        float minimumZ,
        float maximumZ
    ) noexcept
    {
        minimumX_ = minimumX;
        maximumX_ = maximumX;
        minimumZ_ = minimumZ;
        maximumZ_ = maximumZ;
        count_ = 1;
        actors_ = {};

        CharacterActorState& reaper = actors_[0];
        reaper.instanceId = reaperInstanceId;
        reaper.characterId = CharacterId::Reaper;
        reaper.active = true;
        reaper.grounded = true;
        reaper.spawnX = std::clamp(worldSpawnX + 3.60f, minimumX_, maximumX_);
        reaper.spawnY = worldSpawnY;
        reaper.spawnZ = std::clamp(worldSpawnZ - 2.40f, minimumZ_, maximumZ_);
        reaper.spawnYaw = -1.15f;
        reaper.x = reaper.spawnX;
        reaper.y = reaper.spawnY;
        reaper.z = reaper.spawnZ;
        reaper.yaw = reaper.spawnYaw;
        reaper.targetX = reaper.x;
        reaper.targetY = reaper.y;
        reaper.targetZ = reaper.z;
        reaper.activity = CharacterActorActivity::Idle;
        publish();
    }

    void tick(
        float playerX,
        float playerY,
        float playerZ,
        float deltaSeconds
    ) noexcept
    {
        if (!(deltaSeconds > 0.0f) || !std::isfinite(deltaSeconds)) return;
        const float dt = std::min(deltaSeconds, 0.10f);

        for (std::size_t index = 0; index < count_; ++index) {
            CharacterActorState& actor = actors_[index];
            if (!actor.active) continue;
            ++actor.simulationTicks;
            actor.idlePhase += 1.35f * dt;

            const float playerDx = playerX - actor.x;
            const float playerDz = playerZ - actor.z;
            actor.playerDistance = std::sqrt(
                playerDx * playerDx + playerDz * playerDz
            );
            actor.playerInInteractionRange =
                actor.playerDistance <= actor.interactionRadius;

            if (actor.activity == CharacterActorActivity::Walking) {
                const float dx = actor.targetX - actor.x;
                const float dz = actor.targetZ - actor.z;
                const float distance = std::sqrt(dx * dx + dz * dz);
                if (distance <= 0.06f) {
                    actor.x = actor.targetX;
                    actor.y = actor.targetY;
                    actor.z = actor.targetZ;
                    actor.velocityX = 0.0f;
                    actor.velocityZ = 0.0f;
                    actor.movementBlend = 0.0f;
                    actor.activity = CharacterActorActivity::Idle;
                } else {
                    const float invDistance = 1.0f / distance;
                    const float step = std::min(distance, reaperWalkSpeed * dt);
                    const float dirX = dx * invDistance;
                    const float dirZ = dz * invDistance;
                    actor.x = std::clamp(
                        actor.x + dirX * step, minimumX_, maximumX_
                    );
                    actor.z = std::clamp(
                        actor.z + dirZ * step, minimumZ_, maximumZ_
                    );
                    actor.velocityX = dirX * reaperWalkSpeed;
                    actor.velocityZ = dirZ * reaperWalkSpeed;
                    actor.yaw = std::atan2(dirX, dirZ);
                    actor.movementBlend = std::min(
                        1.0f, actor.movementBlend + dt * 7.0f
                    );
                    actor.gaitPhase += 6.6f * dt;
                }
            } else {
                actor.velocityX = 0.0f;
                actor.velocityZ = 0.0f;
                actor.movementBlend = std::max(
                    0.0f, actor.movementBlend - dt * 8.0f
                );
                if (actor.playerDistance <= actor.observationRadius) {
                    actor.yaw = std::atan2(playerDx, playerDz);
                    actor.activity = CharacterActorActivity::ObservingPlayer;
                } else {
                    actor.activity = CharacterActorActivity::Idle;
                }
            }

            (void)playerY; // reserved for future vertical realm/flight actors.
        }
        publish();
    }

    void publish() const noexcept
    {
        CharacterActorPresentationMirror& mirror =
            mutableCharacterActorPresentationMirror();
        mirror.count = std::min(count_, mirror.actors.size());
        for (std::size_t index = 0; index < mirror.count; ++index) {
            mirror.actors[index] = actors_[index];
        }
    }

private:
    std::array<CharacterActorState, capacity> actors_{};
    std::size_t count_ = 0;
    float minimumX_ = -10.0f;
    float maximumX_ = 10.0f;
    float minimumZ_ = -8.0f;
    float maximumZ_ = 8.0f;
};

} // namespace hakui::character
