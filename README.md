# HAKUI

Canonical HAKUI game-engine repository.

HAKUI is the world/body host for the first Spiral Trinity. The engine remains independently playable; Spiral AI connects later through a dedicated HakuiAdapter rather than being fused into renderer, physics, or gameplay code.

Current vertical layer: **L12**. `HakuiWitness` keeps a bounded deterministic
account of session inputs, routing, observations, decisions, mutations, results
and limitations. Records explicitly distinguish observed, inferred and unknown
claims, and F12 exports an immutable `WitnessSnapshot.json` without granting the
observer or cognition any gameplay authority.

## Current Trinity unification

Development branch: `trinity/hakui-unification-v0.1`

The canonical v1.01 gameplay lineage has been promoted from `spiraletech/spiral-ether-tech`. Male and female are now runtime body profiles on one HAKUI skeleton and one gameplay simulation.

```text
HAKUI.exe
   |
   +-- shared world
   +-- shared locomotion
   +-- shared combat
   +-- shared skateboard / BMX
   +-- shared seating / interaction / chat
   +-- shared AvatarRig
          |
          +-- MaleBodyProfile
          `-- FemaleBodyProfile
```

Runtime embodiment controls:

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
AvatarRig
 |- MaleBodyProfile
 `- FemaleBodyProfile
```

XENON is the separate Music Trinity and is not the HAKUI integration bus.

Migration law: preserve gameplay first, unify embodiment second, connect Spiral third.

See `docs/L12_WITNESS.md` for the current self-observation contract and
`docs/L11_ACTION_GATE.md` for the permission and NPC embodiment boundary.
