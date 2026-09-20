# HAKUI L8 — SpiralPresence

L8 makes the existing read-only HAKUI observation link visibly resident in the Black Room.

## Runtime chain

```text
GameRuntime
    -> HakuiSnapshot
    -> HakuiAdapter (read only)
    -> SpiralPresence
    -> HakuiSceneState
    -> DebugWorldRenderer
```

## Authored node

Black Room affordance `1401` is `SPIRAL PRESENCE NODE` with the semantic `Terminal` affordance. Its physical terminal geometry is part of canonical world truth, so the same L6/L7 snapshot/adapter path can observe the node that represents the link.

## Presence panel

The native renderer surfaces:

- `SPIRAL // HAKUI PRESENCE`
- `HAKUI LINK // READ ONLY`
- canonical world ID + simulation step
- player identity + health/stamina
- semantic nearby-object count + snapshot version
- `CORTEX // UNBOUND // L9`

The window HUD also identifies the Spiral Presence when the player approaches its authored node.

## Authority law

`SpiralPresence` accepts only `const HakuiAdapter&`. It owns no `GameRuntime`, player, world, interaction registry, renderer, or Spiral state and exposes no action/mutation API.

L8 is therefore visible embodiment, not AI world control. L9 may bind the conversational Spiral Ether AI/CORTEX behind this resident presence while preserving HAKUI as authoritative reality.
