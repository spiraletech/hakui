#include "avatar/BodyProfileController.hpp"

namespace hakui::avatar {

BodyProfileController::BodyProfileController(const BodyProfileId initial) noexcept
    : active_(initial) {}

BodyProfileId BodyProfileController::activeId() const noexcept {
    return active_;
}

const BodyProfile& BodyProfileController::active() const noexcept {
    return bodyProfile(active_);
}

void BodyProfileController::set(const BodyProfileId id) noexcept {
    active_ = id;
}

void BodyProfileController::toggle() noexcept {
    active_ = active_ == BodyProfileId::Male
        ? BodyProfileId::Female
        : BodyProfileId::Male;
}

bool BodyProfileController::setFromName(const std::string_view name) noexcept {
    if (name == "male") {
        active_ = BodyProfileId::Male;
        return true;
    }
    if (name == "female") {
        active_ = BodyProfileId::Female;
        return true;
    }
    return false;
}

} // namespace hakui::avatar
