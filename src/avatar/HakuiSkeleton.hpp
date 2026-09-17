#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

#include "avatar/AvatarAttachment.hpp"

enum class HakuiSkeletonArchetype {
    None,
    Humanoid,
    ReaperSkeleton
};

struct HakuiBoneDefinition {
    std::string name;
    int parent = -1;
};

// Hakui-owned rig definition.
//
// This class is deliberately engine/runtime agnostic: no SDL, Cal3D, Boost,
// renderer, or physics types are allowed here. Third-party skeletal runtimes
// translate this definition inside optional crystal backends.
class HakuiSkeleton {
public:
    bool buildDefaultHumanoid();
    bool buildReaperSkeleton();

    std::size_t boneCount() const noexcept;
    bool ready() const noexcept;
    HakuiSkeletonArchetype archetype() const noexcept;

    const std::vector<HakuiBoneDefinition>& bones() const noexcept;
    const std::vector<AvatarAttachment>& attachmentSlots() const noexcept;

    int findBone(std::string_view name) const noexcept;

private:
    int addBone(std::string name, int parent);

private:
    HakuiSkeletonArchetype archetype_ = HakuiSkeletonArchetype::None;
    std::vector<HakuiBoneDefinition> bones_;
    std::vector<AvatarAttachment> attachments_;
};
