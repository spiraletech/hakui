# HOME ↔ HAKUI: persistent player bridge

This change connects the native HAKUI L15 client to HOME L25, pinned at
`2a703f0b5a983569253b339454fd36dc7f22ae3c`.

## User behavior

Launch HAKUI, walk with the existing controls, quit normally, and launch again.
The same HOME entity, world revision, position and yaw are restored. The native
client checkpoints movement at up to 10 Hz, saves every five seconds, and commits
and saves once more during shutdown. Sub-millimetre no-ops consume no revision.

The default save is `home.snapshot` under SDL's per-user EtherTech/HAKUI preference
directory. For a separate test slot, set `HAKUI_HOME_SAVE` to an absolute path:

```powershell
$env:HAKUI_HOME_SAVE = "$env:LOCALAPPDATA\EtherTech\HAKUI\bridge-test.snapshot"
.\SPIRAL-OS-HAKUI-ENGINE.exe
```

A malformed or incompatible existing save causes startup to fail with a HOME
error in the log. It is never silently replaced with a new world. A second client
using the same save is refused by an OS-held exclusive lock. Use different save
paths for simultaneous clients. The `.lock` file can remain after exit; its
existence is not the lock, and OS ownership ends automatically on process exit.

## Ownership and mapping

- `HomeSession` owns one `home::VersionedWorld` and its canonical snapshot file.
- `HomeBridge` binds an explicit HOME `EntityId` to HAKUI's player. It projects via
  HOME's `HakuiAdapter`, then submits typed transform consequences through that
  same adapter and HOME's transaction validation.
- The startup session identifies its persistent avatar by the unique archetype
  `hakui.local-player.v1`; ambiguous or missing bindings are rejected. Fresh
  sessions seed the avatar from the existing Black Room spawn.
- HAKUI positions are metres and yaw is radians; HOME uses signed integer
  millimetres and millidegrees. Conversion rejects non-finite coordinates and
  positions beyond ±10,000 metres before narrowing. Pitch and roll remain HOME
  values because this HAKUI player controller only simulates yaw.
- A proposal must match both the last observed and current HOME revision and the
  bound entity. A rejected proposal changes neither canonical state nor revision.
  The client explicitly reloads canonical position after rejection; it does not
  resubmit the stale candidate with a newer revision.
- Movement stays immediate in HAKUI. Quantization is for persistence and does not
  snap live movement to millimetres every frame. Restore clears transient velocity.
- The HOME clock and HAKUI simulation clock remain distinct. This first bridge
  does not claim to synchronize time, NPCs, life stats, seat occupancy, inventory,
  ride mode, or zone geometry. Those values are not added to a competing save.

The initial scene is still the Black Room. This is not a rendered Mission Bay
integration. Transform restore starts with the client's normal transient state;
full activity/seat and riding-session restoration is a later contract.

## Persistence semantics

The file uses HOME's existing v12 snapshot codec. Save writes a sibling `.pending`
file, checks close/write results, then replaces the destination (POSIX rename or
Windows MoveFileExW). Failed writes preserve the prior save. The save slot is held
exclusively for the session. The contract covers normal restart and failure
reporting; it does not promise power-loss durability on every filesystem. A crash
can lose movement since the last successful five-second checkpoint.

## Build and tests

```sh
cmake -S . -B build -DHAKUI_BUILD_NATIVE_CLIENT=OFF -DBUILD_TESTING=ON
cmake --build build --config Release --parallel
ctest --test-dir build -C Release --output-on-failure
```

For offline configuration, supply an already checked out copy of the pinned HOME
revision with `-DFETCHCONTENT_SOURCE_DIR_SPIRAL_HOME=/path/to/SPIRAL-OS-HOME`.
The existing native Windows workflow builds the client, runs these contracts,
and packages the executable. HOME remains an external pinned dependency; its
source is not duplicated into HAKUI.

`hakui.home_bridge` covers actual HAKUI movement, unit conversion, identity,
no-op revisions, pitch/roll preservation, stale submissions, external HOME updates,
wrong entity, NaN/infinity/overflow, failed projection, snapshot round-trip,
exclusive writer ownership, disk write failure, corrupt saves and missing binding.
`hakui.home_restart` launches a writer process and then a fresh reader process;
it checks exact identity/revision, quantized transform, and byte-stable re-save.

The HOME authority string is an in-process caller tag, not authentication. This
bridge exposes no remote mutation endpoint and does not grant model authority.
