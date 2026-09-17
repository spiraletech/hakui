# HAKUI

Canonical HAKUI game-engine repository.

HAKUI is the world/body host for the first Spiral Trinity. The engine remains independently playable; Spiral AI connects later through a dedicated HakuiAdapter rather than being fused into renderer, physics, or gameplay code.

Current vertical layer: **L19**. HAKUI now has deterministic Neeshego manga render
canon on top of L16 identity, L17 runtime presence, and L18 embodiment. Agnathos
resolves to scanline-dominant Raster Veil, Saelis to synthetic halftone ink, and
The Reaper to xerox-crush skeletal presentation. All three are monochrome-only
profiles and can deterministically compose exploration, dialogue, combat windup,
combat impact, realm-shift, and overload frame treatments. The existing L15
natural chat command **“Saelis, come here”** still routes her around furniture,
stops her at a social distance, and turns her toward the player.

## Current Trinity unification

Development branch: `trinity/hakui-unification-v0.1`

The canonical v1.01 gameplay lineage has been promoted from `spiraletech/spiral-ether-tech`. Male and female are runtime body profiles on the HAKUI humanoid rig; L18 adds a separate Reaper skeletal archetype instead of forcing non-human characters through those body profiles. L19 adds presentation grammar without making rendering authoritative over gameplay.

```text
HAKUI.exe
   |
   +-- shared world
   +-- shared locomotion
   +-- shared combat
   +-- shared skateboard / BMX
   +-- shared seating / interaction / chat
   +-- Character Registry
   |      +-- Agnathos
   |      +-- Saelis
   |      `-- The Reaper
   +-- Character Embodiment
   |      +-- Humanoid / body profile
   |      `-- Reaper skeletal rig
   `-- Neeshego Render Grammar
          +-- Raster Veil
          +-- Synthetic Ink
          `-- Xerox Crush
```

Runtime embodiment controls for the humanoid debug avatar remain:

- default profile: `male`
- `F6`: toggle male/female without restarting the world
- startup override: `HAKUI_BODY_PROFILE=male|female`
- canonical state key: `avatar.body_profile`

The dedicated Mannequin Lab remains an isolated rig-science surface. It does not own gameplay authority.

## Canonicalization source

Initial source donors from `spiraletech/spiral-ether-tech`:

- v1.01 gameplay baseline: `codex/hakui-v1.01-skate-embodiment`
- later gameplay lineage: `gpt/hakui-female-player-v1.01`
- male mannequin authority: `gpt/hakui-mannequin-lab-v0.13-silhouette-pass`
- female mannequin authority: `gpt/hakui-female-mannequin-lab-v0.1`

The later gameplay branch is a strict descendant of the v1.01 skate baseline. The canonical import materializes the validated gameplay/rig source passes while intentionally avoiding the old separate compile-time female-game target.

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
Neeshego Manga Render Grammar
```

XENON is the separate Music Trinity and is not the HAKUI integration bus.

Migration law: preserve gameplay first, unify embodiment second, connect Spiral third.

See `docs/L19_MANGA_RENDER_PROFILE.md` for B&W character render grammar and frame
composition rules, `docs/L18_CHARACTER_EMBODIMENT.md` for character rig and
presentation rules, `docs/L17_CHARACTER_REGISTRY.md` for runtime character
instance and lifecycle rules, `docs/L16_CHARACTER_IDENTITY.md` for canonical
character IDs, `docs/L15_NAVIGATION_FOUNDATION.md` for deterministic NPC routing,
`docs/L14_NPC_ACTION_EXECUTOR.md` for the executor boundary,
`docs/L13_INTENT_PROPOSALS.md` for the inert intent contract,
`docs/L12_WITNESS.md` for self-observation, and
`docs/L11_ACTION_GATE.md` for the permission and NPC embodiment boundary.

## HOME persistent player bridge

The native client now loads one HOME-owned player transform, commits movement,
and restores the same entity after restart. See [HOME bridge](docs/HOME_BRIDGE.md)
for save behavior, ownership boundaries and acceptance tests.
