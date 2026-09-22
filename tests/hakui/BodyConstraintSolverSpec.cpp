#include "body/BodyConstraintSolver.hpp"
#include "body/BodyPoseSolver.hpp"

#include <cassert>
#include <cmath>

namespace {

bool near(float a, float b, float epsilon = 0.002f) noexcept
{
    return std::fabs(a - b) <= epsilon;
}

float distance(
    hakui::body::BodyPosePoint a,
    hakui::body::BodyPosePoint b
) noexcept
{
    const float x = a.x - b.x;
    const float y = a.y - b.y;
    const float z = a.z - b.z;
    return std::sqrt(x * x + y * y + z * z);
}

hakui::body::FootGroundSample flatSample(
    const hakui::body::BodyPosePoint& localFoot,
    const hakui::body::BodyPoseState& pose,
    float rootX,
    float rootY,
    float rootZ,
    float groundY
) noexcept
{
    hakui::body::FootGroundSample sample;
    sample.valid = true;
    sample.point = {
        rootX + localFoot.x,
        groundY,
        rootZ + localFoot.z
    };
    sample.normal = {0.0f, 1.0f, 0.0f};
    (void)pose;
    (void)rootY;
    return sample;
}

} // namespace

int main()
{
    hakui::body::BodyPoseSolver poseSolver;
    hakui::body::BodyConstraintSolver constraintSolver;

    PlayerState player;
    player.grounded = true;
    player.movementBlend = 0.0f;

    hakui::body::BodyPoseSolveInput poseInput;
    poseInput.preset = hakui::body::BodyPosePreset::Locomotion;

    const auto nominal = poseSolver.solve(player, poseInput);
    assert(nominal.valid);

    hakui::body::BodyConstraintInput input;
    input.rootX = 0.0f;
    input.rootY = 0.0f;
    input.rootZ = 0.0f;
    input.rootYaw = 0.0f;
    input.deltaTime = 1.0f / 60.0f;
    input.movementBlend = 0.0f;
    input.grounded = true;
    input.enabled = true;
    input.leftGround = flatSample(
        nominal.leftLeg.distal,
        nominal,
        0.0f, 0.0f, 0.0f, 0.0f
    );
    input.rightGround = flatSample(
        nominal.rightLeg.distal,
        nominal,
        0.0f, 0.0f, 0.0f, 0.0f
    );

    hakui::body::BodyConstraintState state;
    auto corrected = constraintSolver.solve(nominal, input, state);

    assert(
        state.leftFoot.phase ==
        hakui::body::FootPlantPhase::Planted
    );
    assert(
        state.rightFoot.phase ==
        hakui::body::FootPlantPhase::Planted
    );
    assert(near(
        distance(
            corrected.leftLeg.proximal,
            corrected.leftLeg.middle
        ),
        input.rig.upperLegLength
    ));
    assert(near(
        distance(
            corrected.leftLeg.middle,
            corrected.leftLeg.distal
        ),
        input.rig.lowerLegLength
    ));

    // Uneven support raises only the sampled foot while preserving exact
    // two-segment leg lengths.
    hakui::body::BodyConstraintState unevenState;
    auto unevenInput = input;
    unevenInput.rightGround.point.y = 0.18f;
    const auto uneven = constraintSolver.solve(
        nominal,
        unevenInput,
        unevenState
    );
    assert(
        uneven.rightLeg.distal.y >
        uneven.leftLeg.distal.y + 0.10f
    );
    assert(near(
        distance(
            uneven.rightLeg.proximal,
            uneven.rightLeg.middle
        ),
        unevenInput.rig.upperLegLength
    ));
    assert(near(
        distance(
            uneven.rightLeg.middle,
            uneven.rightLeg.distal
        ),
        unevenInput.rig.lowerLegLength
    ));

    // A planted target outside the reachable radius must clamp rather than
    // stretch or produce invalid joint coordinates.
    auto unreachableInput = input;
    unreachableInput.rig.plantReleaseDistance = 100.0f;
    unreachableInput.rig.plantReleaseYaw = 10.0f;
    hakui::body::BodyConstraintState unreachableState;
    (void)constraintSolver.solve(
        nominal,
        unreachableInput,
        unreachableState
    );
    unreachableState.leftFoot.phase =
        hakui::body::FootPlantPhase::Planted;
    unreachableState.leftFoot.worldTarget.x -= 3.0f;
    unreachableState.leftFoot.rootYawAtPlant = 0.0f;
    corrected = constraintSolver.solve(
        nominal,
        unreachableInput,
        unreachableState
    );
    assert(unreachableState.leftFoot.reachClamped);
    assert(
        distance(
            corrected.leftLeg.proximal,
            corrected.leftLeg.distal
        ) <=
        unreachableInput.rig.upperLegLength +
        unreachableInput.rig.lowerLegLength
    );
    assert(std::isfinite(corrected.leftLeg.middle.x));
    assert(std::isfinite(corrected.leftLeg.middle.y));
    assert(std::isfinite(corrected.leftLeg.middle.z));

    // A pivot beyond the planted-yaw tolerance releases the contact.
    hakui::body::BodyConstraintState pivotState;
    (void)constraintSolver.solve(nominal, input, pivotState);
    auto pivotInput = input;
    pivotInput.rootYaw = 0.70f;
    (void)constraintSolver.solve(nominal, pivotInput, pivotState);
    assert(
        pivotState.leftFoot.phase !=
        hakui::body::FootPlantPhase::Planted
    );
    assert(
        pivotState.rightFoot.phase !=
        hakui::body::FootPlantPhase::Planted
    );

    // Airborne transition immediately clears all ground locks.
    auto airborneInput = input;
    airborneInput.grounded = false;
    hakui::body::BodyConstraintState airborneState;
    (void)constraintSolver.solve(nominal, input, airborneState);
    const auto airborne = constraintSolver.solve(
        nominal,
        airborneInput,
        airborneState
    );
    assert(airborne.airborne);
    assert(
        airborneState.leftFoot.phase ==
        hakui::body::FootPlantPhase::Swing
    );
    assert(
        airborneState.rightFoot.phase ==
        hakui::body::FootPlantPhase::Swing
    );

    // Disabled correction is an identity path for authored pose studies.
    auto disabledInput = input;
    disabledInput.enabled = false;
    hakui::body::BodyConstraintState disabledState;
    const auto untouched = constraintSolver.solve(
        nominal,
        disabledInput,
        disabledState
    );
    assert(near(
        untouched.rootHeightAbovePlayerBase,
        nominal.rootHeightAbovePlayerBase,
        0.00001f
    ));
    assert(near(
        untouched.leftLeg.distal.x,
        nominal.leftLeg.distal.x,
        0.00001f
    ));
    assert(near(
        untouched.leftLeg.distal.y,
        nominal.leftLeg.distal.y,
        0.00001f
    ));
    assert(near(
        untouched.leftLeg.distal.z,
        nominal.leftLeg.distal.z,
        0.00001f
    ));

    return 0;
}
