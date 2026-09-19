#pragma once

#include <array>

#include "character/CharacterDialogueContent.hpp"

namespace hakui::character {

// Developer-only L25 execution fixture.
//
// These strings and branch effects are deliberately labeled NON-CANON. The
// graph is installed only when HAKUI_NEESHEGO_DIALOGUE_FIXTURE=1 is explicitly
// set. It exists to exercise native authored-dialogue plumbing without making
// story decisions for Neeshego.
inline const AuthoredDialogueGraph& l25DeveloperDialogueFixture() noexcept
{
    static constexpr std::array<AuthoredDialogueNode, 3> nodes{{
        {
            1,
            CharacterId::Reaper,
            "[L25 NON-CANON QA] Dialogue graph online.",
            {{
                {1, "Advance QA branch", 2, {}, {
                    DialogueEffectKind::SetStoryCursor, 900, 1,
                    CharacterRelationshipDisposition::Unassigned
                }},
                {2, "Skip to terminal QA node", 3, {}, {}},
                {},
                {},
            }},
            2,
            false,
        },
        {
            2,
            CharacterId::Reaper,
            "[L25 NON-CANON QA] Authored effect executed.",
            {{
                {3, "Set QA relationship neutral", 3, {}, {
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
            3,
            CharacterId::Reaper,
            "[L25 NON-CANON QA] Terminal node reached.",
            {},
            0,
            true,
        },
    }};

    static const AuthoredDialogueGraph graph{
        CharacterId::Reaper,
        1,
        std::span<const AuthoredDialogueNode>{nodes},
        "developer.l25.noncanon.qa",
    };
    return graph;
}

} // namespace hakui::character
