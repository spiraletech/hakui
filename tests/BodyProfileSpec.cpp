#include "avatar/BodyProfile.hpp"

#include <cassert>

using hakui::avatar::BodyProfileId;
using hakui::avatar::bodyProfile;
using hakui::avatar::femaleBodyProfile;
using hakui::avatar::maleBodyProfile;

int main() {
    const auto& male = maleBodyProfile();
    const auto& female = femaleBodyProfile();

    assert(male.id == BodyProfileId::Male);
    assert(female.id == BodyProfileId::Female);
    assert(&bodyProfile(BodyProfileId::Male) == &male);
    assert(&bodyProfile(BodyProfileId::Female) == &female);

    // These are presentation profiles on one rig, not separate skeletons.
    assert(female.shoulderHalfWidth < male.shoulderHalfWidth);
    assert(female.pelvisSize.x > male.pelvisSize.x);
    assert(female.waistWidthScale < male.waistWidthScale);
    assert(female.thighRadius > male.thighRadius);
    assert(female.calfRadius < male.calfRadius);
    assert(female.handSize.x < male.handSize.x);
    assert(female.footSize.z < male.footSize.z);

    return 0;
}
