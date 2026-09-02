#pragma once

#include <string_view>

namespace hakui::avatar {

enum class BodyProfileId {
    Male,
    Female,
};

struct Vec3f {
    float x{};
    float y{};
    float z{};
};

// Presentation-only embodiment data.
//
// HAKUI owns one authoritative skeleton, pose pipeline, gameplay simulation,
// collision model, ride system and interaction state. A BodyProfile may change
// only the dimensions used to render that shared rig.
struct BodyProfile {
    BodyProfileId id{};
    std::string_view name;

    float shoulderHalfWidth{};
    float shoulderHeight{};
    float elbowOut{};
    float upperArmRadius{};
    float forearmRadius{};

    float thighRadius{};
    float calfRadius{};

    Vec3f handSize{};
    Vec3f footSize{};
    Vec3f pelvisSize{};
    Vec3f waistBridgeSize{};
    Vec3f torsoFrame{};
    Vec3f neckSize{};
    Vec3f headSize{};

    float ribcageWidthScale{};
    float ribcageDepthScale{};
    float waistWidthScale{};
    float waistDepthScale{};
    float clavicleRadius{};
};

[[nodiscard]] const BodyProfile& bodyProfile(BodyProfileId id) noexcept;
[[nodiscard]] const BodyProfile& maleBodyProfile() noexcept;
[[nodiscard]] const BodyProfile& femaleBodyProfile() noexcept;

} // namespace hakui::avatar
