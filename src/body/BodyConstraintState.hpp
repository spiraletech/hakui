#pragma once

#include <cstdint>

#include "body/BodyPoseState.hpp"

namespace hakui::body {

enum class FootPlantPhase : std::uint8_t {
    Swing,
    Planted,
    Releasing,
};

struct FootConstraintState {
    FootPlantPhase phase = FootPlantPhase::Swing;
    BodyPosePoint worldTarget{};
    BodyPosePoint groundNormal{0.0f, 1.0f, 0.0f};
    float rootYawAtPlant = 0.0f;
    float plantAge = 0.0f;
    float releaseBlend = 0.0f;
    float contactError = 0.0f;
    bool reachClamped = false;
};

struct BodyConstraintState {
    bool initialized = false;
    bool wasGrounded = true;

    float pelvisDrop = 0.0f;
    float landingCompression = 0.0f;

    FootConstraintState leftFoot{};
    FootConstraintState rightFoot{};
};

} // namespace hakui::body
