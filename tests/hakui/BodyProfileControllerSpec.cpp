#include "avatar/BodyProfileController.hpp"

#include <cassert>

int main() {
    using hakui::avatar::BodyProfileController;
    using hakui::avatar::BodyProfileId;

    BodyProfileController controller;
    assert(controller.activeId() == BodyProfileId::Male);
    assert(controller.active().name == "male");

    controller.toggle();
    assert(controller.activeId() == BodyProfileId::Female);
    assert(controller.active().name == "female");

    controller.set(BodyProfileId::Male);
    assert(controller.activeId() == BodyProfileId::Male);

    assert(controller.setFromName("female"));
    assert(controller.activeId() == BodyProfileId::Female);

    assert(!controller.setFromName("unknown"));
    assert(controller.activeId() == BodyProfileId::Female);

    return 0;
}
