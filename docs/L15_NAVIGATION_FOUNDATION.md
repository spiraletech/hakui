# HAKUI L15 — Navigation Foundation

L15 gives the L13 `WalkTo` proposal a bounded physical implementation without
allowing a model to invent coordinates or bypass L11 authority. The first
player-facing verb is natural chat: **“Saelis, come here.”** HAKUI resolves the
target from authoritative player state, grants `NpcNavigation` for the current
world step, and passes the request through the L14 executor and L11 gate.

The planner uses the Black Room's canonical `MovementEnvironment`: floor bounds
and the same furniture/wall colliders used by player movement. A deterministic
four-neighbor breadth-first search runs on a 0.5-unit grid with 0.34 units of
agent clearance. It examines at most 4,096 cells and compresses the result into
at most 64 turn waypoints. Blocked, out-of-bounds, malformed, seated, stale, or
unreachable requests fail without mutating resident navigation state.

Saelis follows the resulting path with her accepted female base mannequin and
existing procedural locomotion. She stops 0.9 units from the player's observed
position rather than occupying the same point, faces the player, enters her
bounded observe state, and then resumes her deterministic resident routine.

## Authority chain

`natural player text → WalkTo(player) proposal → current-step NpcNavigation approval → HakuiNpcActionExecutor → HakuiActionGate → NpcManager → HakuiNavigation`

## Invariants

- The player target comes from HAKUI world truth; model text supplies no world
  coordinates.
- Navigation uses the room's collision truth and never owns renderer geometry.
- Route search and storage are fixed-budget and deterministic.
- A proposal remains inert without a matching host approval and capability.
- Seated residents reject navigation; seat ownership is never silently broken.
- Existing female embodiment, locomotion, couch seating, and look-at behavior
  remain on their established authorities.
