#include "body/BodyPoseSolver.hpp"

#include <cassert>
#include <cmath>

namespace {

bool near(float a, float b, float epsilon = 0.0001f) noexcept
{
    return std::fabs(a - b) <= epsilon;
}

} // namespace

int main()
{
    hakui::body::BodyPoseSolver solver;
    PlayerState player;
    player.grounded = true;
    player.movementBlend = 0.0f;

    hakui::body::BodyPoseSolveInput input;
    input.bodyProfile = hakui::avatar::BodyProfileId::Male;
    input.preset = hakui::body::BodyPosePreset::Neutral;
    input.mechanics.leftKneeFlex = 0.18f;
    input.mechanics.rightKneeFlex = 0.18f;
    input.mechanics.leftElbowFlex = 0.10f;
    input.mechanics.rightElbowFlex = 0.10f;

    const auto neutral = solver.solve(player, input);
    assert(neutral.valid);
    assert(neutral.grounded);
    assert(!neutral.airborne);
    assert(neutral.leftLeg.distal.x < 0.0f);
    assert(neutral.rightLeg.distal.x > 0.0f);
    assert(near(
        neutral.leftLeg.distal.y,
        neutral.rightLeg.distal.y
    ));

    input.preset = hakui::body::BodyPosePreset::TPose;
    const auto tpose = solver.solve(player, input);
    assert(tpose.leftArm.distal.x < neutral.leftArm.distal.x);
    assert(tpose.rightArm.distal.x > neutral.rightArm.distal.x);
    assert(tpose.leftArm.distal.y > neutral.leftArm.distal.y);

    input.preset = hakui::body::BodyPosePreset::Crouch;
    input.mechanics.leftKneeFlex = 0.92f;
    input.mechanics.rightKneeFlex = 0.92f;
    input.mechanics.preloadPoseWeight = 0.74f;
    const auto crouch = solver.solve(player, input);
    assert(crouch.pelvisCenter.y < neutral.pelvisCenter.y);
    assert(crouch.leftLeg.middle.y > crouch.leftLeg.distal.y);

    input.preset = hakui::body::BodyPosePreset::Locomotion;
    player.movementBlend = 0.62f;
    player.gaitPhase = 1.2f;
    const auto walking = solver.solve(player, input);
    assert(walking.valid);
    assert(!near(walking.leftLeg.distal.z, walking.rightLeg.distal.z));

    player.grounded = false;
    player.velocityY = 4.0f;
    const auto airborne = solver.solve(player, input);
    assert(airborne.airborne);
    assert(!airborne.grounded);
    assert(airborne.torsoPitch < 0.0f);
    assert(airborne.leftLeg.middle.y > airborne.leftLeg.distal.y);

    return 0;
}
