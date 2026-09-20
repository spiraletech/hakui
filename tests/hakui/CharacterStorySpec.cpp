#include "character/CharacterStoryAuthority.hpp"
#include "core/GameRuntime.hpp"

#include <cassert>

int main()
{
    using namespace hakui::character;

    hakui::GameRuntime runtime;
    CharacterStoryAuthority& story = runtime.characterStory();

    assert(!story.interactionActive());
    assert(story.activeCharacter() == CharacterId::None);

    const CharacterStoryTrack* reaperTrack = story.track(CharacterId::Reaper);
    assert(reaperTrack != nullptr);
    assert(reaperTrack->relationship ==
           CharacterRelationshipDisposition::Unassigned);
    assert(!reaperTrack->cursor.authored);
    assert(reaperTrack->conversationCount == 0);
    assert(reaperTrack->playerTurnCount == 0);

    const CharacterActorState* reaper =
        runtime.independentCharacterActor(CharacterId::Reaper);
    assert(reaper != nullptr);

    runtime.player().x = reaper->x;
    runtime.player().y = reaper->y;
    runtime.player().z = reaper->z;
    runtime.characterActors().tick(
        runtime.player().x,
        runtime.player().y,
        runtime.player().z,
        0.10f
    );
    reaper = runtime.independentCharacterActor(CharacterId::Reaper);
    assert(reaper != nullptr);
    assert(reaper->playerInInteractionRange);

    story.observeActor(*reaper);
    reaperTrack = story.track(CharacterId::Reaper);
    assert(reaperTrack != nullptr);
    assert(reaperTrack->encounterCount == 1);
    assert(reaperTrack->playerInEncounterRange);
    assert(story.latestEvent() != nullptr);
    assert(story.latestEvent()->kind ==
           CharacterStoryEventKind::EncounterEntered);
    assert(story.latestEvent()->authoredContentRequired);

    assert(runtime.beginCharacterStoryInteraction(CharacterId::Reaper));
    assert(runtime.characterStoryInteractionActive());
    assert(story.activeCharacter() == CharacterId::Reaper);
    assert(story.session().instanceId == reaperInstanceId);
    assert(story.latestEvent()->kind ==
           CharacterStoryEventKind::ConversationOpened);

    assert(runtime.submitCharacterStoryPlayerTurn());
    assert(story.session().playerTurnCount == 1);
    reaperTrack = story.track(CharacterId::Reaper);
    assert(reaperTrack != nullptr);
    assert(reaperTrack->playerTurnCount == 1);
    assert(story.latestEvent()->kind ==
           CharacterStoryEventKind::PlayerTurnSubmitted);
    assert(story.latestEvent()->authoredContentRequired);

    assert(reaperTrack->relationship ==
           CharacterRelationshipDisposition::Unassigned);
    assert(!reaperTrack->cursor.authored);
    assert(story.setAuthoredCursor(CharacterId::Reaper, 2, 7));
    assert(story.setAuthoredRelationship(
        CharacterId::Reaper,
        CharacterRelationshipDisposition::Neutral
    ));
    reaperTrack = story.track(CharacterId::Reaper);
    assert(reaperTrack->cursor.authored);
    assert(reaperTrack->cursor.chapterId == 2);
    assert(reaperTrack->cursor.nodeId == 7);
    assert(reaperTrack->relationship ==
           CharacterRelationshipDisposition::Neutral);

    assert(runtime.endCharacterStoryInteraction());
    assert(!runtime.characterStoryInteractionActive());
    assert(story.latestEvent()->kind ==
           CharacterStoryEventKind::ConversationClosed);

    runtime.resetSession();
    reaperTrack = runtime.characterStory().track(CharacterId::Reaper);
    assert(reaperTrack != nullptr);
    assert(reaperTrack->relationship ==
           CharacterRelationshipDisposition::Unassigned);
    assert(!reaperTrack->cursor.authored);
    assert(reaperTrack->conversationCount == 0);
    assert(!runtime.characterStoryInteractionActive());

    return 0;
}
