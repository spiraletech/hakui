# HAKUI L17 — Character Registry / Lifecycle

L17 turns L16's immutable character definitions into first-class runtime
instances without moving physical simulation truth out of HAKUI's established
player and NPC authorities.

The registry is fixed-budget and deterministic. The initial Neeshego roster is:

- **Agnathos** — stable instance `agnathosInstanceId`, bound to the authoritative player.
- **Saelis** — stable instance `saelisInstanceId`, bound to the authored Saelis NPC.
- **The Reaper** — stable instance `reaperInstanceId`, spawned in the runtime roster with no physical actor binding yet.

The Reaper's unbound presence is intentional. L17 establishes that the character
exists in the live runtime and may be spawned/despawned while preserving its
instance identity. L18 will provide non-human skeletal embodiment and rendering;
L17 does not fake that by assigning a human body profile.

## Runtime record

Each `CharacterInstance` contains only:

- stable `CharacterInstanceId`
- canonical `CharacterId`
- physical binding kind (`Player`, `Npc`, or `None`)
- actor id when the binding is an NPC
- lifecycle state (`Spawned` / `Despawned`)
- deterministic lifecycle revision

It deliberately does **not** own transforms, velocity, health, needs, combat,
navigation, animation, rendering, relationships, model context, or HOME save
state.

## Lifecycle contract

`spawn(character)` and `despawn(character)` are idempotent transition requests.
A successful transition increments that instance's lifecycle revision. Repeated
spawn/despawn requests that would not change state are rejected without mutation.
The same `CharacterInstanceId` remains attached to the character across these
transitions.

A full `GameRuntime::resetSession()` restores the authored three-character roster
to `Spawned` using the same deterministic instance ids. A player movement reset
is narrower and does not silently respawn unrelated characters.

## Authority chain

```text
CharacterIdentity (immutable canon)
        |
CharacterRegistry (instance identity + presence)
        |
        +-- Agnathos -> PlayerRuntime physical truth
        +-- Saelis   -> NpcManager physical truth
        `-- Reaper   -> no physical binding until L18
```

## Invariants

- canonical characters have unique stable instance ids
- character lifecycle cannot mutate player/NPC transform state
- player/NPC reset logic cannot rewrite canonical identity
- the Reaper never receives a human `BodyProfile`
- no allocator order is used as character identity
- registry storage is fixed-budget and deterministic
- model inference does not own spawn/despawn truth

## Acceptance

`hakui.character_registry` verifies the full roster, unique instance ids,
bindings, spawn/despawn behavior, lifecycle revision, player-reset isolation and
full-session deterministic restoration.
