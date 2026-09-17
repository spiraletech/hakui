# HAKUI L20 — Neeshego Render Execution

L20 executes the L19 manga render grammar in the native HAKUI client. L19 remains the canonical description of how a character/frame should look; L20 converts that description into bounded render parameters and applies those parameters to the SDL GPU draw path.

## Native execution path

```text
Player / scene state
    |
MangaRenderEvent
    |
L19 MangaFrameTreatment
    |
L20 NeeshegoRenderExecution
    |
NeeshegoRenderPass
    |
SDL GPU render pass
```

The main `hakui` executable now compiles `NeeshegoDebugWorldRenderer.cpp`, a narrow binding shim around the mature `DebugWorldRenderer.cpp`. The Mannequin Lab executables still compile the unmodified renderer directly. This keeps Neeshego presentation scoped to the actual game instead of contaminating rig-science tools.

## Visible behavior

L20 executes the following presentation effects without changing gameplay state:

- character-specific monochrome palette remapping;
- Raster Veil scanlines;
- sparse halftone overlay;
- Reaper xerox streaks and black-field crush;
- manga speed lines during combat emphasis;
- frame-tear bands for Realm Break / impact-capable profiles;
- portrait-cut side bars during dialogue;
- impact / Realm Break frame borders;
- impact flash emphasis.

The implementation intentionally reuses the existing cube vertex buffer and opaque/glass pipelines. No second renderer, shader authority, transform authority, or gameplay clock is introduced.

## Event resolution

The native pass resolves existing HAKUI state into L19 events:

- normal traversal -> `Exploration`;
- local chat input/bubble -> `Dialogue`;
- player combat windup -> `CombatWindup`;
- player/opponent hit pulse -> `CombatImpact`.

`RealmShift` and `Overload` do not invent gameplay triggers in L20. They can be visually validated through the developer-only preview override until their later realm/ability systems own those events.

## Developer visual QA

The following environment variables affect presentation only:

```text
HAKUI_NEESHEGO_PROFILE=agnathos|saelis|reaper
HAKUI_NEESHEGO_EVENT=exploration|dialogue|windup|impact|realm|overload
```

With no override, the pass follows the canonical player character and live scene state. The Reaper override exists only so the xerox execution language can be validated before L21+ gives the Reaper a dedicated world actor/animation path.

## Authority boundary

L20 may read canonical character identity and presentation-relevant scene state. It may remap draw palettes and add screen-space presentation primitives. It may not mutate player/NPC transforms, health, combat state, navigation, camera simulation, character lifecycle, HOME persistence, or AI state.

Presentation noise may evolve per rendered frame, but it never feeds back into deterministic gameplay.

## Acceptance

`hakui.neeshego_render_execution` validates that:

- Agnathos resolves to Raster Monochrome execution with scanlines;
- Saelis resolves to Synthetic Monochrome with reduced corruption;
- The Reaper resolves to Xerox Monochrome with the strongest xerox execution;
- combat impact produces speed lines, frame border, tear bands and impact flash;
- dialogue produces portrait-cut bars;
- realm shift produces Realm Break framing and tear execution;
- an unknown character produces an inactive render plan.

Native Windows CI additionally compiles the real SDL GPU binding shim, catching integration errors that the dependency-free execution spec cannot.
