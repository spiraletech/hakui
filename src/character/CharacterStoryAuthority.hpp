#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "character/CharacterActorAuthority.hpp"

namespace hakui::character {

using StoryChapterId = std::uint32_t;
using StoryNodeId = std::uint32_t;

enum class CharacterRelationshipDisposition : std::uint8_t {
    Unassigned,
    Neutral,
    Allied,
    Opposed,
};

enum class CharacterStoryEventKind : std::uint8_t {
    None,
    EncounterEntered,
    EncounterExited,
    ConversationOpened,
    PlayerTurnSubmitted,
    ConversationClosed,
    CursorAuthored,
    RelationshipAuthored,
};

struct CharacterStoryCursor {
    StoryChapterId chapterId = 0;
    StoryNodeId nodeId = 0;
    bool authored = false;
};

struct CharacterStoryTrack {
    CharacterId characterId = CharacterId::None;
    CharacterRelationshipDisposition relationship =
        CharacterRelationshipDisposition::Unassigned;
    CharacterStoryCursor cursor{};
    std::uint32_t encounterCount = 0;
    std::uint32_t conversationCount = 0;
    std::uint32_t playerTurnCount = 0;
    bool playerInEncounterRange = false;
    std::uint64_t revision = 0;
};

struct CharacterInteractionSession {
    bool active = false;
    CharacterId target = CharacterId::None;
    CharacterInstanceId instanceId = 0;
    std::uint64_t sessionSerial = 0;
    std::uint32_t playerTurnCount = 0;
};

struct CharacterStoryEvent {
    std::uint64_t serial = 0;
    CharacterStoryEventKind kind = CharacterStoryEventKind::None;
    CharacterId characterId = CharacterId::None;
    CharacterInstanceId instanceId = 0;
    CharacterStoryCursor cursor{};
    bool authoredContentRequired = true;
};

// L24 owns deterministic story-interaction bookkeeping only.
//
// It intentionally does NOT invent dialogue, chapter content, relationship
// outcomes, quest rewards, or canon decisions. Those are authored content.
// This authority answers: who is being interacted with, which authored cursor
// is active, how many encounter/conversation turns occurred, and which typed
// story event was emitted.
class CharacterStoryAuthority final {
public:
    static constexpr std::size_t trackCapacity = 8;
    static constexpr std::size_t eventCapacity = 16;

    CharacterStoryAuthority() noexcept { reset(); }

    void reset() noexcept
    {
        tracks_ = {};
        events_ = {};
        trackCount_ = 1;
        eventCount_ = 0;
        nextEventSerial_ = 1;
        nextSessionSerial_ = 1;
        session_ = {};
        tracks_[0].characterId = CharacterId::Reaper;
    }

    [[nodiscard]] const CharacterInteractionSession& session() const noexcept
    {
        return session_;
    }

    [[nodiscard]] bool interactionActive() const noexcept
    {
        return session_.active;
    }

    [[nodiscard]] CharacterId activeCharacter() const noexcept
    {
        return session_.active ? session_.target : CharacterId::None;
    }

    [[nodiscard]] CharacterStoryTrack* track(CharacterId id) noexcept
    {
        for (std::size_t index = 0; index < trackCount_; ++index) {
            if (tracks_[index].characterId == id) return &tracks_[index];
        }
        return nullptr;
    }

    [[nodiscard]] const CharacterStoryTrack* track(CharacterId id) const noexcept
    {
        for (std::size_t index = 0; index < trackCount_; ++index) {
            if (tracks_[index].characterId == id) return &tracks_[index];
        }
        return nullptr;
    }

    [[nodiscard]] std::span<const CharacterStoryEvent> events() const noexcept
    {
        return {events_.data(), eventCount_};
    }

    [[nodiscard]] const CharacterStoryEvent* latestEvent() const noexcept
    {
        return eventCount_ == 0 ? nullptr : &events_[eventCount_ - 1];
    }

    // Encounter entry/exit is proximity truth only. It does not advance an
    // authored chapter or alter relationship disposition.
    void observeActor(const CharacterActorState& actor) noexcept
    {
        CharacterStoryTrack* state = findOrCreate(actor.characterId);
        if (!state) return;

        const bool nowInRange =
            actor.active && actor.playerInInteractionRange;
        if (nowInRange == state->playerInEncounterRange) return;

        state->playerInEncounterRange = nowInRange;
        ++state->revision;
        if (nowInRange) {
            ++state->encounterCount;
            dispatch(
                CharacterStoryEventKind::EncounterEntered,
                *state,
                actor.instanceId,
                true
            );
        } else {
            dispatch(
                CharacterStoryEventKind::EncounterExited,
                *state,
                actor.instanceId,
                false
            );
        }
    }

    bool beginInteraction(
        const CharacterActorAuthority& actors,
        CharacterId id
    ) noexcept
    {
        if (session_.active) return false;
        const CharacterActorState* actor = actors.find(id);
        if (!actor || !actors.canInteract(id)) return false;

        CharacterStoryTrack* state = findOrCreate(id);
        if (!state) return false;

        session_.active = true;
        session_.target = id;
        session_.instanceId = actor->instanceId;
        session_.sessionSerial = nextSessionSerial_++;
        session_.playerTurnCount = 0;

        ++state->conversationCount;
        ++state->revision;
        dispatch(
            CharacterStoryEventKind::ConversationOpened,
            *state,
            actor->instanceId,
            true
        );
        return true;
    }

    bool submitPlayerTurn() noexcept
    {
        if (!session_.active) return false;
        CharacterStoryTrack* state = track(session_.target);
        if (!state) return false;

        ++session_.playerTurnCount;
        ++state->playerTurnCount;
        ++state->revision;
        dispatch(
            CharacterStoryEventKind::PlayerTurnSubmitted,
            *state,
            session_.instanceId,
            true
        );
        return true;
    }

    bool endInteraction() noexcept
    {
        if (!session_.active) return false;
        CharacterStoryTrack* state = track(session_.target);
        if (state) {
            ++state->revision;
            dispatch(
                CharacterStoryEventKind::ConversationClosed,
                *state,
                session_.instanceId,
                false
            );
        }
        session_ = {};
        return true;
    }

    // Explicit authoring seams. Nothing in simulation calls these
    // automatically; a future authored-content layer must make the decision.
    bool setAuthoredCursor(
        CharacterId id,
        StoryChapterId chapterId,
        StoryNodeId nodeId
    ) noexcept
    {
        CharacterStoryTrack* state = findOrCreate(id);
        if (!state) return false;
        state->cursor = {chapterId, nodeId, true};
        ++state->revision;
        dispatch(
            CharacterStoryEventKind::CursorAuthored,
            *state,
            instanceFor(id),
            false
        );
        return true;
    }

    bool setAuthoredRelationship(
        CharacterId id,
        CharacterRelationshipDisposition relationship
    ) noexcept
    {
        CharacterStoryTrack* state = findOrCreate(id);
        if (!state) return false;
        state->relationship = relationship;
        ++state->revision;
        dispatch(
            CharacterStoryEventKind::RelationshipAuthored,
            *state,
            instanceFor(id),
            false
        );
        return true;
    }

private:
    [[nodiscard]] CharacterStoryTrack* findOrCreate(CharacterId id) noexcept
    {
        if (id == CharacterId::None) return nullptr;
        if (CharacterStoryTrack* existing = track(id)) return existing;
        if (trackCount_ >= tracks_.size()) return nullptr;
        CharacterStoryTrack& created = tracks_[trackCount_++];
        created.characterId = id;
        return &created;
    }

    [[nodiscard]] static CharacterInstanceId instanceFor(
        CharacterId id
    ) noexcept
    {
        switch (id) {
            case CharacterId::Agnathos: return agnathosInstanceId;
            case CharacterId::Saelis: return saelisInstanceId;
            case CharacterId::Reaper: return reaperInstanceId;
            case CharacterId::None: return 0;
        }
        return 0;
    }

    void dispatch(
        CharacterStoryEventKind kind,
        const CharacterStoryTrack& state,
        CharacterInstanceId instanceId,
        bool authoredContentRequired
    ) noexcept
    {
        CharacterStoryEvent event{
            nextEventSerial_++,
            kind,
            state.characterId,
            instanceId,
            state.cursor,
            authoredContentRequired,
        };

        if (eventCount_ < events_.size()) {
            events_[eventCount_++] = event;
            return;
        }

        for (std::size_t index = 1; index < events_.size(); ++index) {
            events_[index - 1] = events_[index];
        }
        events_.back() = event;
    }

    std::array<CharacterStoryTrack, trackCapacity> tracks_{};
    std::size_t trackCount_ = 0;
    std::array<CharacterStoryEvent, eventCapacity> events_{};
    std::size_t eventCount_ = 0;
    CharacterInteractionSession session_{};
    std::uint64_t nextEventSerial_ = 1;
    std::uint64_t nextSessionSerial_ = 1;
};

} // namespace hakui::character
