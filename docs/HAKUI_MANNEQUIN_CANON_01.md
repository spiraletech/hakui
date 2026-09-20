# HAKUI MANNEQUIN CANON 01

## Authority

This pass is based on the validated pre-story HAKUI embodiment lineage in
`trinity/hakui-unification-v0.1`, whose mannequin implementation descends from
`spiraletech/spiral-ether-tech@gpt/hakui-mannequin-lab-v0.13-silhouette-pass`.

It intentionally does not depend on the later Neeshego/Reaper story stack.

## Native body pipeline

```text
PlayerMovementController / existing ride mechanics
        |
        v
BodyPoseSolver              <- HAKUI authority
        |
        v
BodyPoseState               <- solved root + joints + orientations
        |
        v
DebugWorldRenderer          <- presentation consumer only
```

`BodyPoseSolver` owns the first explicit body-solution stage. It consumes
deterministic movement state, the existing HAKUI body profile, ground-contact
profile and authored/manual body mechanics. It emits solved pelvis, waist,
torso, clavicle, neck, head, hip, knee, foot, shoulder, elbow and hand targets.

The Mannequin Lab renderer no longer chooses those joint positions when a valid
`BodyPoseState` is present. It connects and draws the already-solved joints.

## Mannequin Lab controls

- `0` — live locomotion
- `WASD` — move/turn in live locomotion
- `Shift` — run
- `Space` — jump
- `1` — neutral authored pose
- `2` — T-pose
- `3` — A-pose
- `4` — crouch
- `5` — existing ollie-pop study
- `Q/E` — authored pelvis yaw
- `Z/X` — authored torso yaw
- `C/V` — authored torso lean
- `[/]` — authored knee flex
- `J` — joint markers
- arrow keys — manual mannequin yaw
- RMB / wheel / `R` — camera

## Scope

This pass preserves the existing skeleton, `AvatarGroundContact`,
`PlayerMovementController`, `RideableMovementController`, body profiles and
SDL3 native executable. Skateboard/BMX types remain compiling, but board
locomotion is not expanded here.

HOME, GUFF, story/dialogue, Reaper actor authority, INOKUI and ENTROKOI are not
part of this pass.

## Acceptance

- HAKUI-MANNEQUIN-LAB builds as a native C++20/SDL3 executable.
- Live mode supports idle, acceleration, walking, sprinting, turning, jumping
  and landing through the existing movement controller.
- Existing ground-contact profiles remain the root/contact reference.
- Body pose is solved by HAKUI before the renderer sees it.
- The renderer consumes solved mannequin joints rather than inventing them.
- Existing authored mannequin poses remain available.
- Board/BMX code continues to compile without expanding board work.
- The next body milestone is procedural locomotion correction / IK, then DUO.
