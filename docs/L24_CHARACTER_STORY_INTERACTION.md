# L24 — Character Story Interaction Authority

L24 adds deterministic story-interaction state without authoring Neeshego canon on behalf of the content layer.

## Authority split

- Agnathos world transform remains owned by `PlayerRuntime`.
- Saelis world transform remains owned by `NpcManager`.
- The Reaper world transform remains owned by L23 `CharacterActorAuthority`.
- L24 `CharacterStoryAuthority` owns only story-facing bookkeeping: encounter entry/exit, active conversation target, player turn counts, an optional authored chapter/node cursor, an optional authored relationship disposition, and typed story events.

## No autonomous canon

L24 deliberately ships with no Reaper dialogue text, plot outcome, alliance/hostility decision, chapter assignment, quest reward, or relationship mutation.

The default relationship is `Unassigned`; the default story cursor is unauthored. Only explicit calls to `setAuthoredCursor` and `setAuthoredRelationship` can change those fields.

## Native client behavior

When the player enters the Reaper's authoritative L23 interaction radius:

1. the HUD exposes a TALK prompt,
2. Interact opens a story session,
3. the camera frames Agnathos + Reaper,
4. locomotion is held while the session is active,
5. Enter opens normal player text input,
6. submitting text records a typed `PlayerTurnSubmitted` story event,
7. the prompt is **not** sent to Cortex and no AI-authored Reaper reply is generated,
8. Cancel closes the interaction and returns to gameplay framing.

While a story session is active, L20/L22 receive a dialogue render event, so manga framing and Reaper dialogue performance are visible without giving the renderer story authority.

## Acceptance

`hakui.character_story` verifies proximity-driven encounter events, Reaper conversation open/close, player-turn accounting, explicit-only authored cursor mutation, explicit-only relationship mutation, and reset to unauthored/unassigned state.
