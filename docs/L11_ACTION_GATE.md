# HAKUI L11 — Permissioned Action Gate

L11 introduces HAKUI's first typed cognition-to-reality boundary. HAKUI remains
authoritative: cortex text is never parsed directly into movement or mutation.

## Action path

```text
frozen HakuiSnapshot
→ typed HakuiActionRequest
→ trusted host HakuiActionGrant
→ HakuiActionGate
→ NpcManager authority
→ audited result
```

Every request carries a nonzero request ID, schema version, observed world step,
source, verb and target resident. Every grant is bound to a source, target,
capability and inclusive world-step window. Missing, mismatched, expired and
stale requests are rejected without mutation.

L11 deliberately exposes only bounded resident-attention verbs:

- `NpcObservePlayer`
- `NpcResumeRoutine`

Both require `NpcAttention`. The resident authority may still reject an allowed
request when it conflicts with physical truth; Saelis cannot be pulled out of a
seat while she owns its canonical occupancy.

## Saelis embodiment repair

Saelis remains NPC `2001` and now explicitly owns the approved `Female`
`BodyProfile`. This does not change the player's selected profile.

The L10 renderer's independent block-body shortcut has been removed. Saelis now
drives the canonical procedural mannequin with the same locomotion constants as
the player base:

- walk blend target `0.62`
- exponential blend response `12`
- gait rate `7.2`
- idle rate `1.8`
- matching stride, arm counter-swing, bob and idle breathing

Her couch pose uses the player base model's seated ground-contact profile and
the same hip/foot targets. Legs are articulated toward the seat pose instead of
being shortened vertically, fixing the L10 sitting-animation snag.

## L11 invariants

- Saelis uses `FemaleBodyProfile`; the player profile remains independent.
- NPC walk/idle/seated animation state is simulation-owned and snapshot-safe.
- Cognition has no raw `GameRuntime`, player, renderer or world pointer.
- Action requests are typed, fresh, target-specific and deny-by-default.
- Host grants cannot be supplied by model prose.
- Seat occupancy remains owned by `BlackRoom` and cannot be bypassed.
- Every evaluated request increments deterministic audit telemetry.
