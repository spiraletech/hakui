# HAKUI L22 — Character Performance Executor

L22 converts the immutable L21 character-performance canon into concrete rig-space pose directives and binds those directives to the native HAKUI renderer.

## Scope

The new `CharacterPoseExecution` contract resolves an authored `CharacterPerformanceFrame` into named rig channels such as spine, neck, skull, jaw, shoulders, arms, hood and cloak. The executor is deterministic and dependency-free. It receives a caller-supplied normalized presentation phase; it does not own time, root motion, transforms, combat, navigation, renderer resources or HOME persistence.

The native Neeshego renderer consumes those channels as a presentation layer on top of the existing HAKUI procedural bodies. Agnathos and Saelis receive visible character-specific ink-shell pose execution in the live client. The Reaper receives a bone-only native stage when `HAKUI_NEESHEGO_PROFILE=reaper` is selected, proving the skeletal animation path without inventing a gameplay transform for his currently unbound L17 character instance.

## Canon motion execution

### Agnathos — Guardian Stillness

Agnathos concentrates motion in the upper spine, neck, skull, hands/arms and Raster eyes. Idle movement is deliberately restrained. Dialogue and combat states increase focused cranial intensity rather than turning him into a generic exaggerated anime rig.

### Saelis — Synthetic Precision

Saelis damps free body sway and uses smaller, cleaner head/torso offsets with authored pose-snap punctuation. Her performance remains humanoid, but the motion grammar is more controlled than Agnathos' organic stillness.

### The Reaper — Reaper Staccato

The Reaper has no human facial-muscle contract and no flesh fallback. L22 emits skull, jaw, cervical/spine, shoulder/arm-bone, hood and cloak channels only. His presentation phase is sampled through deterministic held-frame discontinuities rather than smooth human interpolation. The native preview renderer draws an exposed spine/rib/skull structure with separate jaw and cloth channels; no human torso volume is created.

## Native binding

`src/render/NeeshegoPerformancePass.hpp` runs immediately before the L20 manga overlay pass. It:

- resolves live player performance from movement/dialogue/combat render state,
- resolves Saelis from her deterministic NPC activity,
- executes the corresponding rig-space channels,
- draws character-specific world-space pose accents,
- exposes the Reaper skeletal stage only through the explicit developer profile override,
- then hands the frame to L20 for Raster Veil / Synthetic Ink / Xerox Crush screen treatment.

The mature `DebugWorldRenderer.cpp` remains untouched. The game-only `NeeshegoDebugWorldRenderer.cpp` shim supplies the current `viewProjection` matrix to L22, while the standalone Mannequin Labs continue compiling the original renderer path.

## Developer QA

Use the existing L20 overrides:

```text
HAKUI_NEESHEGO_PROFILE=agnathos|saelis|reaper
HAKUI_NEESHEGO_EVENT=exploration|dialogue|windup|impact|realm|overload
```

For Reaper rig QA, set `HAKUI_NEESHEGO_PROFILE=reaper`. L22 renders the bone-only performance stage beside the current player transform while keeping the Reaper character instance physically unbound.

## Acceptance contract

`hakui.character_performance` now also verifies L22 pose execution:

- Agnathos resolves to the humanoid rig with Raster-eye channels.
- Saelis resolves to the humanoid rig without Raster-eye channels.
- Reaper resolves to `ReaperSkeleton`, `skeletalOnly=true`, no human facial-muscle channel, and explicit Skull/Jaw/Hood/Cloak channels.
- Reaper jaw opening executes as a real joint value.
- Reaper staccato phase produces deterministic discontinuous skull poses.
- `GameRuntime` exposes player, NPC and stable-character-instance pose execution without owning playback.

## Authority law

```text
Character identity       -> what character this is
Character embodiment     -> what rig/body it may use
Character render canon   -> how the character is visually treated
Character performance    -> how the character acts
Character pose executor  -> concrete rig-space directives
Player/NPC authority     -> where the body actually is
Renderer/animation host  -> how those directives are displayed
```

L22 does not silently promote presentation data into simulation truth. A separate future actor-binding layer must give currently unbound characters such as The Reaper an authoritative world transform before they can participate physically in navigation/combat.
