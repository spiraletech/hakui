#include "avatar/BodyProfile.hpp"

namespace hakui::avatar {
namespace {

constexpr BodyProfile kMaleBodyProfile{
    .id = BodyProfileId::Male,
    .name = "male",
    .shoulderHalfWidth = 0.52f,
    .shoulderHeight = 2.04f,
    .elbowOut = 0.075f,
    .upperArmRadius = 0.145f,
    .forearmRadius = 0.112f,
    .thighRadius = 0.188f,
    .calfRadius = 0.145f,
    .handSize = {0.135f, 0.115f, 0.150f},
    .footSize = {0.205f, 0.095f, 0.360f},
    .pelvisSize = {0.54f, 0.20f, 0.32f},
    .waistBridgeSize = {0.46f, 0.13f, 0.30f},
    .torsoFrame = {0.58f, 0.86f, 0.31f},
    .neckSize = {0.145f, 0.205f, 0.145f},
    .headSize = {0.42f, 0.50f, 0.40f},
    .ribcageWidthScale = 1.24f,
    .ribcageDepthScale = 1.10f,
    .waistWidthScale = 0.86f,
    .waistDepthScale = 0.96f,
    .clavicleRadius = 0.095f,
};

constexpr BodyProfile kFemaleBodyProfile{
    .id = BodyProfileId::Female,
    .name = "female",
    .shoulderHalfWidth = 0.46f,
    .shoulderHeight = 2.02f,
    .elbowOut = 0.060f,
    .upperArmRadius = 0.132f,
    .forearmRadius = 0.094f,
    .thighRadius = 0.205f,
    .calfRadius = 0.132f,
    .handSize = {0.115f, 0.095f, 0.135f},
    .footSize = {0.185f, 0.082f, 0.330f},
    .pelvisSize = {0.64f, 0.22f, 0.36f},
    .waistBridgeSize = {0.40f, 0.13f, 0.29f},
    .torsoFrame = {0.54f, 0.84f, 0.30f},
    .neckSize = {0.125f, 0.190f, 0.125f},
    .headSize = {0.39f, 0.47f, 0.37f},
    .ribcageWidthScale = 1.16f,
    .ribcageDepthScale = 1.08f,
    .waistWidthScale = 0.72f,
    .waistDepthScale = 0.92f,
    .clavicleRadius = 0.082f,
};

} // namespace

const BodyProfile& bodyProfile(const BodyProfileId id) noexcept {
    return id == BodyProfileId::Female ? kFemaleBodyProfile : kMaleBodyProfile;
}

const BodyProfile& maleBodyProfile() noexcept {
    return kMaleBodyProfile;
}

const BodyProfile& femaleBodyProfile() noexcept {
    return kFemaleBodyProfile;
}

} // namespace hakui::avatar
