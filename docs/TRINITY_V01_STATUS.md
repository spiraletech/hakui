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

## Validation gate before merge

The branch remains a draft until the packaged native client is manually exercised with both body profiles through the same acceptance loop:

`spawn -> switch body -> walk/sprint/jump -> couch -> Fusion table -> spar -> skateboard -> BMX -> fall/respawn -> switch body again`

## Next engineering layer

After the unified body/world build is accepted, add a read-only `HakuiAdapter` so Spiral can inspect authoritative HAKUI state without becoming a gameplay dependency. XENON remains the separate Music Trinity.
