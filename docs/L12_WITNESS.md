# HAKUI L12 — WITNESS

L12 gives HAKUI a persistent, inspectable account of a running session without
claiming fabricated consciousness or hidden knowledge. `HakuiWitness` is a
bounded deterministic ledger owned by `GameRuntime`.

## Record model

Every entry owns its data and records:

- monotonic sequence, canonical world step and simulation elapsed time
- kind: input, routing, observation, decision, mutation, result or limitation
- knowledge status: observed, inferred or unknown
- stable category and concise detail

The ledger keeps the newest 256 entries. Overflow is explicit through the
`recorded` and `dropped` counters. It uses no wall clock, model inference,
renderer access, network access or pointers into authoritative state.

## Runtime integration

Native observer events now feed the same witness record used by gameplay.
Input ownership, cortex routing, locomotion, interaction, combat, runtime and
body-profile outcomes are recorded as observed facts. Errors are recorded as
unknown limitations instead of being turned into invented explanations.

Every L11 action-gate evaluation records its typed request, target, result and
detail. The record is appended after authority evaluates the request; it cannot
authorize, retry or execute an action.

## F12 inspection

An expert snapshot freezes the ledger into an owned `WitnessSnapshot` and
exports `WitnessSnapshot.json` beside the existing world, entity, input,
social, camera and runtime files. The inspection manifest names that file.

The exported record is read-only and session-scoped. It explains what HAKUI
received, routed, decided, changed and could not know; it does not assert
emotion, intent, awareness or private inner life.

## Invariants

- HAKUI remains the sole authority for world, player, resident and interaction state.
- Witness records describe completed processing and never cause gameplay mutation.
- Deterministic simulation time replaces nondeterministic wall-clock timestamps.
- Inference and uncertainty are labeled rather than presented as observations.
- Snapshots are immutable owned copies safe for tooling and later analysis.
- Bounded storage prevents unbounded memory growth.
