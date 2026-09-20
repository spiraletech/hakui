#pragma once

#include "avatar/BodyProfile.hpp"
#include "body/BodyPoseState.hpp"
#include "player/PlayerState.hpp"
#include "player/RideableMovementController.hpp"

namespace hakui::body {

struct BodyPoseSolveInput {
    avatar::BodyProfileId bodyProfile = avatar::BodyProfileId::Male;
    BodyPosePreset preset = BodyPosePreset::Locomotion;
    RideBodyMechanicsState mechanics{};
};

class BodyPoseSolver final {
public:
    [[nodiscard]] BodyPoseState solve(
        const PlayerState& player,
        const BodyPoseSolveInput& input
    ) const noexcept;
};

} // namespace hakui::body
