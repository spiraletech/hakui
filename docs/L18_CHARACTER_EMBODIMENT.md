# HAKUI L18 — Character Embodiment

L18 turns L16 identity + L17 runtime presence into explicit embodiment canon without collapsing HAKUI's authority boundaries.

## Canonical profiles

- **Agnathos** — HAKUI humanoid rig, male body profile, Raster Veil visual state, B&W Raster Veil surface language.
- **Saelis** — HAKUI humanoid rig, female body profile, synthetic B&W surface language.
- **The Reaper** — dedicated non-human Reaper skeleton rig, no human body profile, no human face slot, no hair slot, xerox/grunge surface language.

`CharacterEmbodimentProfile` is immutable canon. It selects rig archetype, optional body profile, default visual state, surface language, material key and animation-set key. It does not own transform, health, movement, combat, renderer resources, animation playback, or HOME persistence.

## Reaper rig

`HakuiSkeleton::buildReaperSkeleton()` creates a distinct 27-bone topology with explicit skull, jaw, hood, cloak and chain anchors. This is not a male/female humanoid placeholder. The rig intentionally exposes skeletal/garment/weapon attachment slots and omits human face/hair attachment slots.

The Reaper remains a live L17 character instance with no player/NPC actor binding. L18 establishes the body topology and presentation contract; later world embodiment can bind this rig to a dedicated non-human actor without contaminating the humanoid player/NPC path.

## Runtime access

`GameRuntime` exposes canonical embodiment lookup for the player, NPCs and stable `CharacterInstanceId` values. Session reset preserves the authored character roster and resolves the same embodiment canon deterministically.

## Acceptance

- `hakui.character_embodiment` verifies Agnathos/Saelis/Reaper profile contracts and runtime lookup.
- `hakui.reaper_rig` verifies dedicated Reaper topology, non-human attachment semantics, no face/hair slots, and clean rebuild back to the humanoid rig.
