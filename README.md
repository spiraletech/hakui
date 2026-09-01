# HAKUI

Canonical HAKUI game-engine repository.

HAKUI is the world/body host for the first Spiral Trinity. The engine remains independently playable; Spiral AI connects through XENON rather than being fused into renderer, physics, or gameplay code.

## Canonicalization source

The initial engine lineage is being promoted from `spiraletech/spiral-ether-tech`:

- gameplay baseline: `codex/hakui-v1.01-skate-embodiment`
- latest female gameplay lineage: `gpt/hakui-female-player-v1.01`
- male mannequin authority: `gpt/hakui-mannequin-lab-v0.13-silhouette-pass`
- female mannequin authority: `gpt/hakui-female-mannequin-lab-v0.1`

The male and female builds are not separate engines. They are body/presentation profiles on one canonical rig and one gameplay simulation.

## Trinity direction

```text
Spiral AI
   |
 XENON
   |
HakuiAdapter
   |
 HAKUI
   |
AvatarRig
 |- MaleBodyProfile
 `- FemaleBodyProfile
```

Migration rule: preserve gameplay behavior first, unify embodiment second, connect Spiral third.
