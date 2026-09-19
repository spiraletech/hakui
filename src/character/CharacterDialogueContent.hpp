#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include "character/CharacterStoryAuthority.hpp"

namespace hakui::character {

using DialogueNodeId = std::uint32_t;
using DialogueChoiceId = std::uint32_t;

enum class DialogueConditionKind : std::uint8_t {
    Always,
    RelationshipEquals,
    StoryCursorEquals,
    MinimumPlayerTurns,
    MinimumConversations,
};

struct DialogueCondition {
    DialogueConditionKind kind = DialogueConditionKind::Always;
    CharacterRelationshipDisposition relationship =
        CharacterRelationshipDisposition::Unassigned;
    StoryChapterId chapterId = 0;
    StoryNodeId storyNodeId = 0;
    std::uint32_t threshold = 0;
};

enum class DialogueEffectKind : std::uint8_t {
    None,
    SetStoryCursor,
    SetRelationship,
};

struct DialogueEffect {
    DialogueEffectKind kind = DialogueEffectKind::None;
    StoryChapterId chapterId = 0;
    StoryNodeId storyNodeId = 0;
    CharacterRelationshipDisposition relationship =
        CharacterRelationshipDisposition::Unassigned;
};

struct AuthoredDialogueChoice {
    DialogueChoiceId id = 0;
    std::string_view text{};
    DialogueNodeId targetNodeId = 0;
    DialogueCondition condition{};
    DialogueEffect effect{};
};

struct AuthoredDialogueNode {
    static constexpr std::size_t choiceCapacity = 4;

    DialogueNodeId id = 0;
    CharacterId speaker = CharacterId::None;
    std::string_view text{};
    std::array<AuthoredDialogueChoice, choiceCapacity> choices{};
    std::size_t choiceCount = 0;
    bool terminal = false;
};

struct AuthoredDialogueGraph {
    CharacterId characterId = CharacterId::None;
    DialogueNodeId entryNodeId = 0;
    std::span<const AuthoredDialogueNode> nodes{};
    std::string_view authoringKey{};
};

struct AvailableDialogueChoices {
    std::array<const AuthoredDialogueChoice*,
               AuthoredDialogueNode::choiceCapacity> choices{};
    std::size_t count = 0;
};

enum class DialogueSelectionResult : std::uint8_t {
    Rejected,
    Advanced,
    Completed,
};

class CharacterDialogueCatalog final {
public:
    static constexpr std::size_t graphCapacity = 8;

    bool install(AuthoredDialogueGraph graph) noexcept
    {
        if (!validate(graph)) return false;

        for (std::size_t index = 0; index < count_; ++index) {
            if (graphs_[index].characterId == graph.characterId) {
                graphs_[index] = graph;
                return true;
            }
        }
        if (count_ >= graphs_.size()) return false;
        graphs_[count_++] = graph;
        return true;
    }

    [[nodiscard]] const AuthoredDialogueGraph* graph(
        CharacterId characterId
    ) const noexcept
    {
        for (std::size_t index = 0; index < count_; ++index) {
            if (graphs_[index].characterId == characterId) {
                return &graphs_[index];
            }
        }
        return nullptr;
    }

    [[nodiscard]] std::size_t size() const noexcept { return count_; }

    [[nodiscard]] static const AuthoredDialogueNode* findNode(
        const AuthoredDialogueGraph& graph,
        DialogueNodeId id
    ) noexcept
    {
        for (const AuthoredDialogueNode& node : graph.nodes) {
            if (node.id == id) return &node;
        }
        return nullptr;
    }

    [[nodiscard]] static bool validate(
        const AuthoredDialogueGraph& graph
    ) noexcept
    {
        if (graph.characterId == CharacterId::None ||
            graph.entryNodeId == 0 ||
            graph.nodes.empty() ||
            graph.authoringKey.empty() ||
            !findNode(graph, graph.entryNodeId)) {
            return false;
        }

        for (std::size_t index = 0; index < graph.nodes.size(); ++index) {
            const AuthoredDialogueNode& node = graph.nodes[index];
            if (node.id == 0 ||
                node.speaker == CharacterId::None ||
                node.text.empty() ||
                node.choiceCount > node.choices.size()) {
                return false;
            }
            for (std::size_t other = index + 1;
                 other < graph.nodes.size(); ++other) {
                if (graph.nodes[other].id == node.id) return false;
            }
            if (node.terminal && node.choiceCount != 0) return false;
            if (!node.terminal && node.choiceCount == 0) return false;

            for (std::size_t choiceIndex = 0;
                 choiceIndex < node.choiceCount; ++choiceIndex) {
                const AuthoredDialogueChoice& choice =
                    node.choices[choiceIndex];
                if (choice.id == 0 || choice.text.empty() ||
                    choice.targetNodeId == 0 ||
                    !findNode(graph, choice.targetNodeId)) {
                    return false;
                }
                for (std::size_t other = choiceIndex + 1;
                     other < node.choiceCount; ++other) {
                    if (node.choices[other].id == choice.id) return false;
                }
            }
        }
        return true;
    }

private:
    std::array<AuthoredDialogueGraph, graphCapacity> graphs_{};
    std::size_t count_ = 0;
};

struct CharacterDialogueSession {
    bool active = false;
    bool complete = false;
    CharacterId characterId = CharacterId::None;
    DialogueNodeId nodeId = 0;
    std::uint32_t selectionCount = 0;
};

class CharacterDialogueRuntime final {
public:
    void reset() noexcept { session_ = {}; }

    [[nodiscard]] const CharacterDialogueSession& session() const noexcept
    {
        return session_;
    }

    [[nodiscard]] bool active() const noexcept { return session_.active; }
    [[nodiscard]] bool complete() const noexcept { return session_.complete; }

    bool begin(
        const CharacterDialogueCatalog& catalog,
        const CharacterStoryAuthority& story,
        CharacterId characterId
    ) noexcept
    {
        if (session_.active ||
            !story.interactionActive() ||
            story.activeCharacter() != characterId) {
            return false;
        }
        const AuthoredDialogueGraph* graph = catalog.graph(characterId);
        if (!graph) return false;
        const AuthoredDialogueNode* entry =
            CharacterDialogueCatalog::findNode(*graph, graph->entryNodeId);
        if (!entry) return false;

        session_.active = true;
        session_.complete = entry->terminal;
        session_.characterId = characterId;
        session_.nodeId = entry->id;
        session_.selectionCount = 0;
        return true;
    }

    [[nodiscard]] const AuthoredDialogueNode* currentNode(
        const CharacterDialogueCatalog& catalog
    ) const noexcept
    {
        if (!session_.active) return nullptr;
        const AuthoredDialogueGraph* graph =
            catalog.graph(session_.characterId);
        return graph
            ? CharacterDialogueCatalog::findNode(*graph, session_.nodeId)
            : nullptr;
    }

    [[nodiscard]] AvailableDialogueChoices availableChoices(
        const CharacterDialogueCatalog& catalog,
        const CharacterStoryAuthority& story
    ) const noexcept
    {
        AvailableDialogueChoices result;
        const AuthoredDialogueNode* node = currentNode(catalog);
        if (!node || node->terminal) return result;

        const CharacterStoryTrack* track =
            story.track(session_.characterId);
        if (!track) return result;

        for (std::size_t index = 0;
             index < node->choiceCount &&
             result.count < result.choices.size();
             ++index) {
            const AuthoredDialogueChoice& choice = node->choices[index];
            if (conditionSatisfied(choice.condition, *track)) {
                result.choices[result.count++] = &choice;
            }
        }
        return result;
    }

    DialogueSelectionResult selectChoiceByOrdinal(
        const CharacterDialogueCatalog& catalog,
        CharacterStoryAuthority& story,
        std::size_t ordinal
    ) noexcept
    {
        if (!session_.active || session_.complete || ordinal == 0) {
            return DialogueSelectionResult::Rejected;
        }
        const AvailableDialogueChoices available =
            availableChoices(catalog, story);
        if (ordinal > available.count) {
            return DialogueSelectionResult::Rejected;
        }

        const AuthoredDialogueChoice& choice =
            *available.choices[ordinal - 1];
        if (!applyEffect(choice.effect, story, session_.characterId)) {
            return DialogueSelectionResult::Rejected;
        }

        const AuthoredDialogueGraph* graph =
            catalog.graph(session_.characterId);
        if (!graph) return DialogueSelectionResult::Rejected;
        const AuthoredDialogueNode* target =
            CharacterDialogueCatalog::findNode(*graph, choice.targetNodeId);
        if (!target) return DialogueSelectionResult::Rejected;

        session_.nodeId = target->id;
        ++session_.selectionCount;
        session_.complete = target->terminal;
        return session_.complete
            ? DialogueSelectionResult::Completed
            : DialogueSelectionResult::Advanced;
    }

private:
    [[nodiscard]] static bool conditionSatisfied(
        const DialogueCondition& condition,
        const CharacterStoryTrack& track
    ) noexcept
    {
        switch (condition.kind) {
            case DialogueConditionKind::Always:
                return true;
            case DialogueConditionKind::RelationshipEquals:
                return track.relationship == condition.relationship;
            case DialogueConditionKind::StoryCursorEquals:
                return track.cursor.authored &&
                    track.cursor.chapterId == condition.chapterId &&
                    track.cursor.nodeId == condition.storyNodeId;
            case DialogueConditionKind::MinimumPlayerTurns:
                return track.playerTurnCount >= condition.threshold;
            case DialogueConditionKind::MinimumConversations:
                return track.conversationCount >= condition.threshold;
        }
        return false;
    }

    static bool applyEffect(
        const DialogueEffect& effect,
        CharacterStoryAuthority& story,
        CharacterId characterId
    ) noexcept
    {
        switch (effect.kind) {
            case DialogueEffectKind::None:
                return true;
            case DialogueEffectKind::SetStoryCursor:
                return story.setAuthoredCursor(
                    characterId, effect.chapterId, effect.storyNodeId
                );
            case DialogueEffectKind::SetRelationship:
                return story.setAuthoredRelationship(
                    characterId, effect.relationship
                );
        }
        return false;
    }

    CharacterDialogueSession session_{};
};

} // namespace hakui::character
