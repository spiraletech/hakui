# L25 — Authored Dialogue Content

L25 adds the deterministic content graph that L24 intentionally left empty.

## Scope

L25 introduces authored dialogue nodes, player choices, conditions, explicit effects, branching, terminal nodes, and a fixed-budget dialogue catalog/runtime.

The engine still does **not** write Neeshego canon. Content must be explicitly installed as an `AuthoredDialogueGraph`.

## Content model

Each authored graph declares:

- the canonical character it belongs to,
- an entry node,
- stable node and choice IDs,
- authored speaker text,
- up to four choices per node,
- optional deterministic conditions,
- optional explicit story effects,
- terminal nodes.

Supported conditions are relationship equality, authored story-cursor equality, minimum player-turn count, minimum conversation count, and unconditional availability.

Supported effects are setting an authored story cursor or setting an authored relationship disposition. These are the same explicit authoring seams introduced by L24; L25 merely lets authored content invoke them.

## Authority law

`CharacterDialogueCatalog` owns immutable graph references. `CharacterDialogueRuntime` owns only the active node/selection session.

It does not own character transforms, render state, NPC simulation, or story proximity. L24 remains the story bookkeeping authority.

## Native QA

Production runtime starts with **zero dialogue graphs installed**.

For native execution testing only, setting:

`HAKUI_NEESHEGO_DIALOGUE_FIXTURE=1`

installs a graph whose lines are visibly marked `L25 NON-CANON QA`. This fixture is not Neeshego story canon.

When a graph is installed, the Reaper story interaction can display the current authored line and numbered available choices. Player input selects an authored branch deterministically and never routes through Cortex.

## Acceptance

`hakui.character_dialogue` verifies graph validation, installation, conditional choice resolution, deterministic branching, explicit cursor/relationship effects, terminal nodes, session reset, and rejection of invalid targets.
