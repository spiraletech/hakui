# HAKUI L21 — Character Performance

L21 adds deterministic authored motion and expression language for the Neeshego cast. It builds on L16 identity, L17 runtime instances, L18 embodiment, L19 manga render grammar, and L20 native render execution.

## Canonical performance grammar

- **Agnathos — Guardian Stillness**
  - restrained movement, low-amplitude idle motion, asymmetrical hand/arm language
  - Raster Blank / Raster Focused / Raster Strained expression states
  - Raster-eye intensity is a dedicated authored channel
  - realm shift and overload increase cranial intensity and snap without changing gameplay authority

- **Saelis — Synthetic Precision**
  - controlled stride, low asymmetry, smooth gesture language
  - Synthetic Neutral / Warm / Alert expression states
  - dialogue is warmer and more open; combat/realm events become tighter and more precise

- **The Reaper — Reaper Staccato**
  - non-human stop/start motion, strong pose snap, high phase discontinuity and asymmetry
  - Reaper Still / Open Jaw / Rattle expression states
  - no human facial-muscle contract exists
  - expression is skull, jaw, hood, spine and skeletal pose language only

## Performance events

`CharacterPerformanceEvent` exposes:

- Idle
- Locomotion
- Dialogue
- CombatReady
- CombatImpact
- RealmShift
- Overload

L19 `MangaRenderEvent` values deterministically map into the matching performance event where appropriate.

## Output contract

`CharacterPerformanceFrame` contains renderer/animation-facing authored values:

- head pitch / roll
- torso pitch / roll
- left / right arm bias
- stride scale
- idle amplitude
- gesture weight
- jaw openness
- cranial intensity
- pose snap
- staccato weight

These values are bounded pose/expression directives. They do **not** own transforms, root motion, health, combat, navigation, animation playback, GPU resources, character lifecycle or HOME persistence.

## Runtime access

`GameRuntime` now resolves performance canon for:

- the authoritative player
- NPC actors
- stable `CharacterInstanceId` values

The Reaper therefore has a complete non-human performance contract even while his world-actor binding remains a separate later step.

## Acceptance

`hakui.character_performance` verifies:

- each canonical character resolves to a distinct motion grammar
- Agnathos uses Raster expression channels
- Saelis uses synthetic expression channels
- Reaper has no human facial-muscle contract
- Reaper jaw/rattle/staccato states remain skeletal-only
- render events map to the expected performance events
- runtime lookup survives session reset
