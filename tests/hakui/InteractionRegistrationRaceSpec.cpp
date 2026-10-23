#include <cassert>
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include "interaction/InteractionService.hpp"

namespace {
class ReplacingTarget final : public hakui::Interactable {
public:
    explicit ReplacingTarget(hakui::EntityId id) : id_(id) {}
    hakui::EntityId interactionId() const noexcept override { return id_; }
    std::vector<hakui::InteractionOption> interactionOptions(hakui::EntityId) const override {
        if (onOptions) onOptions();
        return {{hakui::InteractionVerb::Open, "Open"}};
    }
    hakui::InteractionResult interact(const hakui::InteractionRequest&) override {
        ++executions;
        return {true, "opened", {{"world.door.open", true}}};
    }
    mutable std::function<void()> onOptions;
    int executions = 0;
private:
    hakui::EntityId id_;
};
void replacement_during_options_cannot_execute_stale_target() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto oldTarget = std::make_shared<ReplacingTarget>(100);
    auto newTarget = std::make_shared<ReplacingTarget>(100);
    assert(service.registerTarget(oldTarget));
    std::vector<std::string> topics;
    bus.subscribe([&](const spiral::Signal& s) { topics.push_back(s.topic); });
    bool replaced = false;
    oldTarget->onOptions = [&]() {
        if (replaced) return;
        replaced = true;
        assert(service.unregisterTarget(100));
        assert(service.registerTarget(newTarget));
    };
    const hakui::InteractionRequest request{7, 100, hakui::InteractionVerb::Open, {}};
    const auto result = service.interact(request);
    assert(!result.handled);
    assert(oldTarget->executions == 0);
    assert(newTarget->executions == 0);
    assert(topics.size() == 2);
    assert(topics[0] == "interaction.request");
    assert(topics[1] == "interaction.target_changed");
    assert(service.interact(request).handled);
    assert(newTarget->executions == 1);
}
void removal_during_options_cannot_execute_unregistered_target() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<ReplacingTarget>(200);
    assert(service.registerTarget(target));
    bool removed = false;
    target->onOptions = [&]() {
        if (removed) return;
        removed = true;
        assert(service.unregisterTarget(200));
    };
    const hakui::InteractionRequest request{7, 200, hakui::InteractionVerb::Open, {}};
    assert(!service.interact(request).handled);
    assert(target->executions == 0);
    assert(service.targetCount() == 0);
}
void unrelated_registration_change_does_not_cancel_dispatch() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<ReplacingTarget>(300);
    auto unrelated = std::make_shared<ReplacingTarget>(301);
    assert(service.registerTarget(target));
    bool added = false;
    target->onOptions = [&]() {
        if (added) return;
        added = true;
        assert(service.registerTarget(unrelated));
    };
    const hakui::InteractionRequest request{7, 300, hakui::InteractionVerb::Open, {}};
    assert(service.interact(request).handled);
    assert(target->executions == 1);
}
void same_instance_reregistration_revokes_original_dispatch() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<ReplacingTarget>(400);
    assert(service.registerTarget(target));
    bool changed = false;
    target->onOptions = [&]() {
        if (changed) return;
        changed = true;
        assert(service.unregisterTarget(400));
        assert(service.registerTarget(target));
    };
    const hakui::InteractionRequest request{7, 400, hakui::InteractionVerb::Open, {}};
    assert(!service.interact(request).handled);
    assert(target->executions == 0);
    assert(service.interact(request).handled);
    assert(target->executions == 1);
}
void stable_registration_remains_executable() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<ReplacingTarget>(500);
    assert(service.registerTarget(target));
    const hakui::InteractionRequest request{7, 500, hakui::InteractionVerb::Open, {}};
    assert(service.interact(request).handled);
    assert(target->executions == 1);
}

void removed_target_cannot_offer_stale_menu() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<ReplacingTarget>(600);
    assert(service.registerTarget(target));
    target->onOptions = [&]() { assert(service.unregisterTarget(600)); };
    assert(service.options(7, 600).empty());
    assert(service.targetCount() == 0);
}
void replaced_target_cannot_offer_old_menu() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto original = std::make_shared<ReplacingTarget>(601);
    auto replacement = std::make_shared<ReplacingTarget>(601);
    assert(service.registerTarget(original));
    original->onOptions = [&]() {
        assert(service.unregisterTarget(601));
        assert(service.registerTarget(replacement));
    };
    assert(service.options(7, 601).empty());
    assert(service.options(7, 601).size() == 1);
}
void same_instance_aba_cannot_offer_stale_menu() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<ReplacingTarget>(602);
    assert(service.registerTarget(target));
    bool changed = false;
    target->onOptions = [&]() {
        if (changed) return;
        changed = true;
        assert(service.unregisterTarget(602));
        assert(service.registerTarget(target));
    };
    assert(service.options(7, 602).empty());
    assert(service.options(7, 602).size() == 1);
}
void unrelated_registration_does_not_hide_menu() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<ReplacingTarget>(603);
    auto unrelated = std::make_shared<ReplacingTarget>(604);
    assert(service.registerTarget(target));
    target->onOptions = [&]() { assert(service.registerTarget(unrelated)); };
    assert(service.options(7, 603).size() == 1);
}
void stable_registration_offers_menu() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<ReplacingTarget>(605);
    assert(service.registerTarget(target));
    assert(service.options(7, 605).size() == 1);
    assert(service.options(7, 999).empty());
}
}
int main() {
    replacement_during_options_cannot_execute_stale_target();
    removal_during_options_cannot_execute_unregistered_target();
    unrelated_registration_change_does_not_cancel_dispatch();
    same_instance_reregistration_revokes_original_dispatch();
    stable_registration_remains_executable();
    removed_target_cannot_offer_stale_menu();
    replaced_target_cannot_offer_old_menu();
    same_instance_aba_cannot_offer_stale_menu();
    unrelated_registration_does_not_hide_menu();
    stable_registration_offers_menu();
}
