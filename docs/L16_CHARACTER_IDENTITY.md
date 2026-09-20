# HAKUI L16 — Character Identity Layer

L16 introduces first-class character canon without moving deterministic game
state out of the authorities that already own it. A player or resident now
carries a stable `CharacterId`; immutable `CharacterIdentity` data resolves that
ID into authored canon such as name, species, faction, realm alignment, body
profile, face identity, default visual state and embodiment class.

The initial canon set is deliberately small:

- **Agnathos** — playable human guardian, Spiral Grove aligned, male body
  profile, `agnathos.base-face.v1`, default visual state `RasterVeil`.
- **Saelis** — synthetic Spiral companion, Spiral OS aligned, female body
  profile, `saelis.base-face.v1`, default visual state `SaelisDefault`.
- **The Reaper** — non-human skeletal entity aligned to the death boundary. It
  has no human body profile or human face identity and defaults to
  `ReaperSkeleton`.

## Authority contract

Character identity is immutable canon. It does **not** own position, health,
needs, locomotion, combat state, navigation, animation, relationships,
persistence, renderer state or model context. Those remain on their existing
HAKUI/HOME authorities.

`PlayerState` carries `CharacterId::Agnathos`. The current authored resident,
Saelis, is rebound to `CharacterId::Saelis` by `GameRuntime` at boot and after a
full deterministic reset. The Reaper is canonically defined but is not spawned
by L16; character lifecycle and a runtime character registry are reserved for
L17.

## Runtime lookup

`GameRuntime::playerIdentity()` resolves the current playable identity.
`GameRuntime::npcIdentity(id)` resolves an authored resident identity without
copying canon into NPC simulation state.

This keeps the distinction explicit:

`character canon -> CharacterId -> deterministic player/NPC state`

rather than:

`character prose/model state -> gameplay authority`

## Acceptance invariants

- Agnathos resolves to the male humanoid Raster Veil identity.
- Saelis resolves to the female humanoid companion identity.
- The Reaper resolves to a non-human skeletal identity with no body profile.
- Unknown IDs and keys resolve to no identity.
- Player reset deterministically restores the Agnathos binding.
- Full session reset deterministically restores both Agnathos and Saelis
  bindings.
- No renderer, SDL, network, persistence or model dependency enters the
  gameplay layer.

The contract is guarded by `hakui.character_identity`.

## L17 boundary

L17 may add a real character registry/lifecycle owner that can spawn multiple
named character instances. L16 intentionally stops before that line: it makes
characters addressable and canonical, but does not yet make identity a second
world-state authority.
