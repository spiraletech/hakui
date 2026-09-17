#include "avatar/HakuiSkeleton.hpp"

#include <cassert>
#include <string_view>
#include <unordered_set>

int main()
{
    HakuiSkeleton skeleton;
    assert(!skeleton.ready());
    assert(skeleton.archetype() == HakuiSkeletonArchetype::None);

    assert(skeleton.buildReaperSkeleton());
    assert(skeleton.ready());
    assert(skeleton.archetype() == HakuiSkeletonArchetype::ReaperSkeleton);
    assert(skeleton.boneCount() == 27);

    // Reaper is explicitly skeletal. There is a skull/jaw and garment/chain
    // anchor topology, but no human face or hair attachment slots.
    assert(skeleton.findBone("Skull") >= 0);
    assert(skeleton.findBone("Jaw") >= 0);
    assert(skeleton.findBone("HoodAnchor") >= 0);
    assert(skeleton.findBone("CloakAnchor") >= 0);
    assert(skeleton.findBone("ChainAnchor") >= 0);
    assert(skeleton.findBone("Head") == -1);

    std::unordered_set<std::string_view> slots;
    bool hasSkull = false;
    bool hasJaw = false;
    bool hasHood = false;
    bool hasCloak = false;
    bool hasChain = false;
    bool hasWeapon = false;
    bool hasFace = false;
    bool hasHair = false;

    for (const AvatarAttachment& attachment : skeleton.attachmentSlots()) {
        assert(slots.insert(attachment.slot).second);
        assert(skeleton.findBone(attachment.bone) >= 0);
        hasSkull |= attachment.category == AttachmentCategory::Skull;
        hasJaw |= attachment.category == AttachmentCategory::Jaw;
        hasHood |= attachment.category == AttachmentCategory::Hood;
        hasCloak |= attachment.category == AttachmentCategory::Cloak;
        hasChain |= attachment.category == AttachmentCategory::Chain;
        hasWeapon |= attachment.category == AttachmentCategory::Weapon;
        hasFace |= attachment.category == AttachmentCategory::Face;
        hasHair |= attachment.category == AttachmentCategory::Hair;
    }

    assert(hasSkull && hasJaw && hasHood && hasCloak && hasChain && hasWeapon);
    assert(!hasFace);
    assert(!hasHair);

    // Rebuilding as a humanoid must fully replace the non-human topology.
    assert(skeleton.buildDefaultHumanoid());
    assert(skeleton.archetype() == HakuiSkeletonArchetype::Humanoid);
    assert(skeleton.boneCount() == 23);
    assert(skeleton.findBone("Head") >= 0);
    assert(skeleton.findBone("Skull") == -1);

    return 0;
}
