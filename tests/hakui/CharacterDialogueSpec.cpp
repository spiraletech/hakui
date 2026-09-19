#include "character/CharacterDialogueContent.hpp"
#include "core/GameRuntime.hpp"

#include <array>
#include <cassert>

int main()
{
    using namespace hakui::character;

    static constexpr std::array<AuthoredDialogueNode, 3> nodes{{
        {
            10,
            CharacterId::Reaper,
            "fixture.entry",
            {{
                {1, "take authored cursor branch", 20, {}, {
                    DialogueEffectKind::SetStoryCursor, 4, 2,
                    CharacterRelationshipDisposition::Unassigned
                }},
                {2, "terminal branch", 30, {
                    DialogueConditionKind::MinimumConversations,
                    CharacterRelationshipDisposition::Unassigned,
                    0, 0, 1
                }, {}},
                {},
                {},
            }},
            2,
            false,
        },
        {
            20,
            CharacterId::Reaper,
            "fixture.effect",
            {{
                {3, "author relationship neutral", 30, {}, {
                    DialogueEffectKind::SetRelationship, 0, 0,
                    CharacterRelationshipDisposition::Neutral
                }},
                {},
                {},
                {},
            }},
            1,
            false,
        },
        {
            30,
            CharacterId::Reaper,
            "fixture.terminal",
            {},
            0,
            true,
        },
    }};

    static const AuthoredDialogueGraph graph{
        CharacterId::Reaper,
        10,
        std::span<const AuthoredDialogueNode>{nodes},
        "test.l25.fixture",
    };

    assert(CharacterDialogueCatalog::validate(graph));

    CharacterDialogueCatalog catalog;
    assert(catalog.size() == 0);
    assert(catalog.install(graph));
    assert(catalog.size() == 1);
    assert(catalog.graph(CharacterId::Reaper) != nullptr);

    hakui::GameRuntime runtime;
    assert(runtime.dialogueCatalog().size() == 0);
    assert(runtime.installCharacterDialogueGraph(graph));
    assert(runtime.dialogueCatalog().size() == 1);
    assert(!runtime.characterDialogueActive());

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
    assert(runtime.beginCharacterStoryInteraction(CharacterId::Reaper));

    const CharacterStoryTrack* track =
        runtime.characterStory().track(CharacterId::Reaper);
    assert(track != nullptr);
    assert(track->relationship ==
           CharacterRelationshipDisposition::Unassigned);
    assert(!track->cursor.authored);

    assert(runtime.beginCharacterDialogue(CharacterId::Reaper));
    const AuthoredDialogueNode* current =
        runtime.currentCharacterDialogueNode();
    assert(current != nullptr);
    assert(current->id == 10);
    assert(current->text == "fixture.entry");

    AvailableDialogueChoices choices =
        runtime.availableCharacterDialogueChoices();
    assert(choices.count == 2);
    assert(choices.choices[0]->id == 1);
    assert(choices.choices[1]->id == 2);

    const DialogueSelectionResult first =
        runtime.selectCharacterDialogueChoice(1);
    assert(first == DialogueSelectionResult::Advanced);
    current = runtime.currentCharacterDialogueNode();
    assert(current != nullptr);
    assert(current->id == 20);

    track = runtime.characterStory().track(CharacterId::Reaper);
    assert(track != nullptr);
    assert(track->cursor.authored);
    assert(track->cursor.chapterId == 4);
    assert(track->cursor.nodeId == 2);
    assert(track->relationship ==
           CharacterRelationshipDisposition::Unassigned);

    const DialogueSelectionResult second =
        runtime.selectCharacterDialogueChoice(1);
    assert(second == DialogueSelectionResult::Completed);
    assert(runtime.characterDialogueActive());
    assert(runtime.characterDialogueComplete());
    current = runtime.currentCharacterDialogueNode();
    assert(current != nullptr);
    assert(current->id == 30);

    track = runtime.characterStory().track(CharacterId::Reaper);
    assert(track->relationship ==
           CharacterRelationshipDisposition::Neutral);

    assert(runtime.endCharacterStoryInteraction());
    assert(!runtime.characterDialogueActive());
    assert(!runtime.characterStoryInteractionActive());

    // Session resets do not delete authored content registration.
    runtime.resetSession();
    assert(runtime.dialogueCatalog().size() == 1);
    assert(!runtime.characterDialogueActive());

    // Invalid content is rejected: missing target node.
    static constexpr std::array<AuthoredDialogueNode, 1> brokenNodes{{
        {
            1,
            CharacterId::Reaper,
            "broken",
            {{
                {1, "missing target", 999, {}, {}},
                {}, {}, {},
            }},
            1,
            false,
        },
    }};
    const AuthoredDialogueGraph broken{
        CharacterId::Reaper,
        1,
        std::span<const AuthoredDialogueNode>{brokenNodes},
        "test.invalid",
    };
    assert(!CharacterDialogueCatalog::validate(broken));
    assert(!runtime.installCharacterDialogueGraph(broken));

    return 0;
}
