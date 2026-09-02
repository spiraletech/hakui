#pragma once

#include "avatar/BodyProfile.hpp"

#include <string_view>

namespace hakui::avatar {

// Runtime presentation selector for the canonical HAKUI rig.
//
// This controller never owns gameplay, collision, locomotion, combat or pose
// authority. It selects only which BodyProfile is used to present the shared
// avatar skeleton.
class BodyProfileController final {
public:
    explicit BodyProfileController(BodyProfileId initial = BodyProfileId::Male) noexcept;

    [[nodiscard]] BodyProfileId activeId() const noexcept;
    [[nodiscard]] const BodyProfile& active() const noexcept;

    void set(BodyProfileId id) noexcept;
    void toggle() noexcept;

    // Stable CLI/config seam for the native client. Unknown values are rejected
    // rather than silently changing embodiment.
    [[nodiscard]] bool setFromName(std::string_view name) noexcept;

private:
    BodyProfileId active_;
};

} // namespace hakui::avatar
