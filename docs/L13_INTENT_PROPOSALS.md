# HAKUI L13 — Intent Proposals

L13 converts only explicit Cortex response envelopes into typed proposal data:

`HAKUI_PROPOSE <LookAt|WalkTo|Sit|Talk|Inspect> <actor-id> <target-id> <argument>`

Ordinary dialogue remains dialogue. Unknown verbs, zero identities, malformed
lines and oversized arguments are rejected. A response yields at most eight
proposals, preventing unbounded model output from expanding runtime work.

Proposals carry a host-issued sequence, verb, actor, target, bounded argument
and original evidence. Parsing owns no `GameRuntime`, `NpcManager`, world,
player, renderer or interaction reference. Therefore parsing cannot move,
seat, speak for, inspect through, or redirect Saelis.

Accepted envelopes are written to L12 WITNESS as **inferred inert proposals**.
L11 remains the only cognition-to-reality approval boundary. L14 may map an
approved subset into action requests, but L13 deliberately provides no executor.

## Invariants

- Model prose is never executable.
- Only the five named verbs enter typed proposal data.
- Proposal parsing performs no gameplay mutation.
- L12 labels proposals as inferred, not observed world truth.
- L11 grants remain mandatory for any future execution.
