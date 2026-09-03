# HAKUI L10 — Deterministic NPC Simulation

L10 introduces the first resident simulation authority in HAKUI without making gameplay depend on a language model, GGUF file, Spiral cortex process, renderer, SDL, or network service.

## Authority topology

```text
GameRuntime
├── HakuiWorldState
├── PlayerRuntime
├── NpcManager
└── InteractionRegistry
```

`NpcManager` owns resident state. The world owns authored affordances and seat occupancy. The player remains a separate authority. Spiral can observe resident copies through `HakuiSnapshot`/`HakuiAdapter`, but L10 defines no AI action verb.

## First resident

L10 authors one deterministic resident:

```text
NPC 2001 // SAELIS
```

Saelis begins in the Black Room and follows a repeatable model-independent routine:

```text
home
→ walk to Void Couch
→ reserve a real couch seat
→ rest
→ release seat
→ south navigation bypass
→ walk to Spiral Presence
→ observe
→ north navigation bypass
→ return home
→ idle
→ repeat
```

When the player enters a small notice radius while Saelis is not seated, she temporarily faces/observes the player, then resumes the existing routine. This reaction is deterministic simulation logic, not generated dialogue.

## Needs

The first resident carries bounded simulation needs:

- hunger
- energy
- social
- comfort
- fun

Needs evolve from activity. Couch rest restores energy/comfort; player observation restores social; Spiral-node observation adds fun. These values are objective game-state inputs for later life-sim planning, not prompt-only personality text.

## Shared seat truth

`BlackRoom` now exposes explicit `reserveSeat()` / `releaseSeat()` operations. NPC seating uses the same `SeatAnchor::occupied` truth as player seating, so HAKUI cannot silently seat a resident and player in the same slot through two unrelated occupancy models.

## Snapshot / adapter

`HakuiSnapshot` schema v2 adds owned `HakuiNpcSnapshot` values. Old snapshots remain frozen after later resident simulation.

The read-only adapter adds:

```text
hakui.inspect_npcs
```

This exposes resident truth without movement, dialogue, schedule-edit, spawn, despawn, seat, inventory, or world-mutation authority.

The L9 cortex observation packet is extended with copied resident state. This does not create an AI control path; it only lets a connected cortex know which residents currently exist and what deterministic state they are in.

## Native presentation

The SDL client advances accepted simulation deltas through `GameRuntime::advanceWorld()` so NPC and world clocks share one accepted timeline. The debug renderer receives a read-only resident span and draws a lightweight Saelis mannequin. Final animation/nameplate/avatar systems remain later layers.

## L10 invariants

- HAKUI NPC life functions with the cortex process absent.
- Invalid world deltas cannot mutate NPC state.
- Resident seats use canonical BlackRoom occupancy.
- Session reset releases resident seats and restores authored NPC state.
- Player proximity reaction is deterministic and bounded.
- Resident snapshots are owned frozen copies.
- Adapter/cortex observation remains read-only.
- No L10 code executes an AI response as a game action.

L11 is the first layer allowed to introduce a permissioned `HakuiActionGate`. Until that exists, cognition may observe and speak, but it cannot command residents or world state.
