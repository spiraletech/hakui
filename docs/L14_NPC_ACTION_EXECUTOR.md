# HAKUI L14 — NPC Action Executor

L14 introduces the first proposal-to-action translator without weakening the
L11 authority boundary. A proposal alone remains inert. Execution requires a
host-created approval matching proposal ID, actor, target, capability and the
current deterministic world step.

The first executable shape is intentionally narrow:

`Saelis LookAt player → NpcObservePlayer → HakuiActionGate → NpcManager`

The native host grants only `NpcAttention`, only to Saelis (`2001`), only for
player target `1`, and only for the current world step. The L11 gate performs
its own independent source, target, capability, freshness and authority checks.

`WalkTo`, `Sit`, `Talk`, and `Inspect` remain typed L13 proposals but return
`UnsupportedVerb`; L14 does not fake navigation, seat authority, speech
ownership or inspection semantics before those systems exist.

## Invariants

- Cortex text cannot create an approval or grant.
- Proposal and approval identities must match exactly.
- Stale approvals cannot execute.
- The executor translates; L11 authorizes; `NpcManager` owns the mutation.
- Denied and executed attempts remain visible through L12 WITNESS.
- Saelis cannot be interrupted when her physical authority rejects attention.
