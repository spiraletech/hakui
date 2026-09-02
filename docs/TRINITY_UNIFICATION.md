# HAKUI Trinity Unification v0.1

## Decision

`spiraletech/hakui` is the canonical HAKUI repository. `spiraletech/spiral-ether-tech` is the historical/source-donor repository for the current migration.

## Source authorities

- gameplay baseline: `codex/hakui-v1.01-skate-embodiment` (`d967d3e1002b69cbc81f9b75c97f1a9d139b1c44`)
- female gameplay lineage: `gpt/hakui-female-player-v1.01` (`c03da5a02b3ac288186c6468b08b5ae2e8ce55a9`)
- male rig authority: `gpt/hakui-mannequin-lab-v0.13-silhouette-pass` (`5a4ef3f1a0e7c670657fab42d25653f06b9db243`)
- female rig authority: `gpt/hakui-female-mannequin-lab-v0.1` (`688d22d0b85472f2454a34d7f4cc629cc1e7a92b`)

The female gameplay branch is a descendant of the v1.01 skate-embodiment branch. Its female player work is implemented as a presentation-shell patch while the underlying locomotion, ride physics, combat, seating, chat and interactions remain shared.

## Core law

One engine. One skeleton. One pose pipeline. One gameplay simulation. Multiple presentation profiles.

A `BodyProfile` may change visual rig dimensions, but must not own gameplay state, collision authority, ride physics, combat semantics, world state, or interaction behavior.

## Migration sequence

1. Promote body dimensions from the mannequin branches into typed runtime `BodyProfile` data.
2. Import the v1.01 gameplay source into this repository without changing behavior.
3. Replace compile-time female-shell patching with runtime `BodyProfileId` selection.
4. Keep Mannequin Lab as the permanent rig-science executable using the same profiles.
5. Prove both profiles against the same gameplay acceptance loop.
6. Add `HakuiAdapter` as the XENON boundary.
7. Connect Spiral AI only after the unified HAKUI build is stable.

## First acceptance target

```text
HAKUI.exe
  -> choose male or female profile
  -> same world
  -> same camera
  -> same walk/sprint/jump
  -> same couch/table interactions
  -> same combat
  -> same skateboard
  -> same BMX
  -> same respawn
```

The only intentional difference at this milestone is body presentation.
