# HAKUI

Canonical HAKUI game-engine repository.

HAKUI is the world/body host for the first Spiral Trinity. The engine remains independently playable; Spiral AI connects later through a dedicated HakuiAdapter rather than being fused into renderer, physics, or gameplay code.

Current vertical layer: **L23**. HAKUI now carries named Neeshego characters from identity through embodiment, manga render grammar, native render execution, authored performance language, concrete rig-space pose execution, and real character-actor authority. Agnathos remains owned by PlayerRuntime, Saelis remains owned by NpcManager, and The Reaper now has an independent authoritative world transform instead of existing only as an unbound registry presence or developer preview. The existing L15 natural chat command **“Saelis, come here”** still routes her around furniture, stops her at a social distance, and turns her toward the player.

## Current Trinity unification

Development branch: `trinity/hakui-unification-v0.1`

The canonical v1.01 gameplay lineage was promoted from `spiraletech/spiral-ether-tech`. Male and female remain runtime body profiles on the HAKUI humanoid rig; The Reaper owns a separate skeletal archetype. L19 defines manga presentation grammar, L20 executes it in the native SDL GPU renderer, L21 defines how each character performs, L22 converts that performance into rig-space pose channels, and L23 gives independent cast members authoritative world-actor state without duplicating PlayerRuntime/NpcManager ownership.

```text
HAKUI.exe
   |
   +-- shared world / locomotion / combat / riding / interaction / chat
   |
   +-- Character Registry
   |      +-- Agnathos
   |      +-- Saelis
   |      `-- The Reaper
   |
   +-- Character Embodiment
   |      +-- Humanoid
   |      |    +-- MaleBodyProfile
   |      |    `-- FemaleBodyProfile
   |      `-- ReaperSkeleton
   |
   +-- Neeshego Render Grammar
   |      +-- Raster Veil
   |      +-- Synthetic Ink
   |      `-- Xerox Crush
   |
   +-- Neeshego Native Render Execution
   |      +-- monochrome palette remap
   |      +-- scanline / halftone / xerox overlays
   |      `-- manga panel / impact / realm treatment
   |
   +-- Character Performance Canon
   |      +-- Guardian Stillness
   |      +-- Synthetic Precision
   |      `-- Reaper Staccato
   |
   +-- Character Pose Executor
   |      +-- spine / neck / skull / jaw
   |      +-- shoulders / arm bias
   |      +-- Raster-eye intensity
   |      `-- Reaper hood / cloak channels
   |
   `-- Character Actor Authority
          +-- authored spawn anchor
          +-- world transform / facing
          +-- walk target / locomotion state
          +-- player proximity / interaction range
          +-- renderer presentation mirror
          `-- Reaper world-space skeletal binding
```

## Runtime embodiment controls

Humanoid debug-avatar controls remain:

- default profile: `male`
- `F6`: toggle male/female without restarting the world
- startup override: `HAKUI_BODY_PROFILE=male|female`
- canonical state key: `avatar.body_profile`

Developer Neeshego visual/performance QA overrides:

- `HAKUI_NEESHEGO_PROFILE=agnathos|saelis|reaper`
- `HAKUI_NEESHEGO_EVENT=exploration|dialogue|windup|impact|realm|overload`

`HAKUI_NEESHEGO_PROFILE=reaper` keeps the original L22 bone-only QA stage available. In normal gameplay L23 renders The Reaper from the independent actor authority at his actual authored world transform; the QA override suppresses that L23 draw to avoid a duplicate skeleton.

The dedicated Mannequin Lab remains an isolated rig-science surface. It compiles the unmodified debug renderer and does not inherit the game-only Neeshego performance/render shim.

## Canonicalization source

Initial source donors from `spiraletech/spiral-ether-tech`:

- v1.01 gameplay baseline: `codex/hakui-v1.01-skate-embodiment`
- later gameplay lineage: `gpt/hakui-female-player-v1.01`
- male mannequin authority: `gpt/hakui-mannequin-lab-v0.13-silhouette-pass`
- female mannequin authority: `gpt/hakui-female-mannequin-lab-v0.1`

## Build contracts

Dependency-free gameplay/core contracts:

```sh
cmake -S . -B build -DHAKUI_BUILD_NATIVE_CLIENT=OFF -DHAKUI_ENABLE_IMVU_CAL3D=OFF -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build --build-config Release --output-on-failure
```

Windows native client:

```sh
cmake -S . -B build -DHAKUI_BUILD_NATIVE_CLIENT=ON -DHAKUI_ENABLE_IMVU_CAL3D=OFF -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build --build-config Release --output-on-failure
```

GitHub Actions publishes a Windows x64 package from the Trinity branch containing `SPIRAL-OS-HAKUI-ENGINE.exe`, `SDL3.dll`, and `START_HERE.txt`.

Implementation/acceptance status is tracked in `docs/TRINITY_V01_STATUS.md`.

## Trinity direction

```text
Spiral AI
   |
HakuiAdapter
   |
 HAKUI
   |
Character Registry
   |
Character Embodiment
   |
Manga Render Grammar
   |
Native Render Execution
   |
Character Performance Canon
   |
Character Pose Executor
   |
Character Actor Authority
```

XENON is the separate Music Trinity and is not the HAKUI integration bus.

Migration law: preserve gameplay first, unify embodiment second, connect Spiral third.

See `docs/L23_CHARACTER_ACTOR_AUTHORITY.md` for independent cast world authority,
`docs/L22_CHARACTER_POSE_EXECUTOR.md` for rig-space performance execution,
`docs/L21_CHARACTER_PERFORMANCE.md` for authored character motion/expression canon,
`docs/L20_RENDER_EXECUTION.md` for native manga execution,
`docs/L19_MANGA_RENDER_PROFILE.md` for B&W character render grammar,
`docs/L18_CHARACTER_EMBODIMENT.md` for character rig rules,
`docs/L17_CHARACTER_REGISTRY.md` for runtime character lifecycle,
`docs/L16_CHARACTER_IDENTITY.md` for canonical character IDs,
and `docs/HOME_BRIDGE.md` for persistent HOME-owned player transform behavior.
