#pragma once

#include "body/BodyConstraintState.hpp"
#include "body/BodyPoseState.hpp"
#include "body/BodyRigMetrics.hpp"

namespace hakui::body {

// World-space support sample supplied by HAKUI's contact/environment boundary.
// The constraint layer consumes the result; it does not query HOME or renderers.
struct FootGroundSample {
    bool valid = false;
    BodyPosePoint point{};
    BodyPosePoint normal{0.0f, 1.0f, 0.0f};
};

struct BodyConstraintInput {
    BodyRigMetrics rig = canonicalHumanoidRigMetrics();

    FootGroundSample leftGround{};
    FootGroundSample rightGround{};

    float rootX = 0.0f;
    float rootY = 0.0f;
    float rootZ = 0.0f;
    float rootYaw = 0.0f;

    float deltaTime = 0.0f;
    float movementBlend = 0.0f;

    bool grounded = true;
    bool enabled = true;
};

class BodyConstraintSolver final {
public:
    [[nodiscard]] BodyPoseState solve(
        const BodyPoseState& nominalPose,
        const BodyConstraintInput& input,
        BodyConstraintState& state
    ) const noexcept;
};

} // namespace hakui::body
