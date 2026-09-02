# HAKUI Trinity v0.1 Status

## Implemented

- canonical repository established at `spiraletech/hakui`
- v1.01 gameplay lineage imported from the pinned historical donor commit
- male v0.13 and female v0.1 mannequin measurements promoted into immutable `BodyProfile` data
- one runtime `BodyProfileController` owns embodiment selection
- one native `HAKUI.exe` can switch male/female presentation with `F6`
- startup profile override through `HAKUI_BODY_PROFILE=male|female`
- active profile published as `avatar.body_profile`
- renderer consumes the active profile for shoulders, arms, hands, pelvis, torso taper, clavicles, neck, head, thighs, calves, and feet
- gameplay authority remains shared: locomotion, combat, skateboard, BMX, seating, interaction, chat, world and collision are not forked by body profile
- Linux and Windows dependency-free contracts are validated in CI
- Windows native build/test/package workflow produces a runnable package
- manual body-profile smoke test passed visually; the extended Void Couch regression was isolated to duplicated seat geometry and repaired without changing avatar/runtime logic

## L2 — World regression stabilization

L2 is implemented and guarded by `hakui.world_regression`.

The deterministic regression contract now verifies:

- every authored world primitive is finite, positive-sized, and has a valid repeat count
- every expanded repeated primitive produces sane bounds
- world affordance IDs remain unique and their volumes remain valid
- the player spawn remains on valid floor and inside the recovery affordance
- the Void Couch remains inside its intended visual envelope instead of growing an accidental right-side tail
- the Void Couch expands to exactly six furniture instances: base, back, two arms, and two seat cushions
- the couch movement collider remains bounded to the intended couch footprint
- the couch remains a two-seat semantic affordance
- both seat anchors resolve inside the same couch affordance
- sit/leave state is reversible, releases seat reservations, and permits another player to sit afterward

The L2 contract is compiled and executed as part of the dependency-free gameplay CI on Linux and Windows.

## Validation gate before merge

The branch remains a draft until the packaged native client is manually exercised with both body profiles through the same acceptance loop:

`spawn -> switch body -> walk/sprint/jump -> couch -> Fusion table -> spar -> skateboard -> BMX -> fall/respawn -> switch body again`

## Next engineering layer

L3 extracts `GameRuntime` so `HakuiApp` becomes orchestration rather than the owner of growing gameplay state. After the runtime/world-state layers are established, HAKUI can expose a read-only `HakuiAdapter` to Spiral Ether AI without making AI a gameplay dependency. XENON remains the separate Music Trinity.
