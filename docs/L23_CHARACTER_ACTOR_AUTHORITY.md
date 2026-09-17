# HAKUI L23 — Character Actor Authority

L23 turns canonical cast members that are neither the player nor an NPC resident into real world actors.

## Authority law

HAKUI already has two mature transform owners:

- `PlayerRuntime` owns the playable Agnathos transform.
- `NpcManager` owns resident transforms such as Saelis.

L23 does **not** duplicate those transforms. `CharacterActorAuthority` owns only independent canonical actors whose `CharacterRegistry` binding is `None`. The first such actor is The Reaper.

## Reaper world state

The Reaper now has:

- stable `reaperInstanceId`
- authoritative `x/y/z/yaw`
- authored spawn anchor
- walk target and velocity
- idle/gait presentation phases
- `Idle`, `Walking`, and `ObservingPlayer` activity
- deterministic facing toward the player inside observation range
- authoritative player distance and interaction-range truth
- reset-to-authored-spawn behavior

The initial Black Room anchor is derived from the room spawn and clamped to the room bounds. No human NPC body profile is introduced.

## Renderer bridge

`CharacterActorAuthority` publishes a copy-only native presentation mirror after authoritative mutation. The renderer can inspect that mirror but cannot mutate simulation state.

`NeeshegoCharacterActorPass` consumes the mirror and binds each actor transform to the L22 pose executor. For The Reaper this means the existing skeletal-only rig is drawn at the actor's actual authoritative world transform instead of at L22's temporary developer preview offset.

The L22 `HAKUI_NEESHEGO_PROFILE=reaper` preview remains available and intentionally suppresses the L23 actor draw in that one QA mode to avoid a duplicate skeleton.

## Reaper body law

The L23 renderer path keeps the existing canon unchanged:

- no skin/flesh torso
- no human face shell
- no hair slot
- no human facial-muscle contract
- skull, jaw, vertebrae, ribs, bony shoulders/arms, hood, and cloak only

## Determinism

Actor walking uses fixed authored speed and caller-provided delta time clamped to HAKUI's maximum simulation step. Facing and proximity are derived only from authoritative player/actor coordinates.

## Acceptance

`hakui.character_performance` additionally verifies:

1. Reaper has one active independent actor with the stable canonical instance ID.
2. The actor begins at its authored spawn anchor.
3. `requestIndependentCharacterWalkTo` advances the authoritative transform and locomotion channels.
4. Player proximity sets interaction-range truth in actor state.
5. The renderer presentation mirror matches authoritative actor state.
6. Session reset restores the authored actor spawn.

## Next seam

L24 can build actual character interaction/story authority on top of L23: target selection, conversation ownership, relationship state, encounter triggers, and chapter/event dispatch without moving those rules into rendering.
