#pragma once

namespace hakui::body {

// Canonical HAKUI kinematic dimensions.
//
// These values describe the physical reach of the shared humanoid rig. They
// are intentionally separate from avatar::BodyProfile, which is presentation
// geometry only.
struct BodyRigMetrics {
    float upperLegLength = 0.76f;
    float lowerLegLength = 0.74f;
    float footTargetAboveSurface = 0.14f;

    float maxPelvisDrop = 0.30f;
    float legReachMargin = 0.01f;

    float plantAcquireHeight = 0.10f;
    float plantAcquireDistance = 0.22f;
    float plantReleaseHeight = 0.16f;
    float plantReleaseDistance = 0.34f;
    float plantReleaseYaw = 0.45f;
    float plantReleaseSpeed = 8.0f;

    float pelvisAcquireRate = 18.0f;
    float pelvisRecoveryRate = 7.0f;

    float landingCompression = 0.12f;
    float landingRecoverySpeed = 0.85f;
};

[[nodiscard]] constexpr BodyRigMetrics canonicalHumanoidRigMetrics() noexcept
{
    return {};
}

} // namespace hakui::body
