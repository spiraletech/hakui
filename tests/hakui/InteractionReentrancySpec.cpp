#include <algorithm>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "interaction/InteractionService.hpp"

namespace {
class CountingTarget final : public hakui::Interactable {
public:
    explicit CountingTarget(hakui::EntityId id) : id_(id) {}
    hakui::EntityId interactionId() const noexcept override { return id_; }
    std::vector<hakui::InteractionOption> interactionOptions(hakui::EntityId) const override {
        return {{hakui::InteractionVerb::Inspect, "Inspect"}};
    }
    hakui::InteractionResult interact(const hakui::InteractionRequest&) override {
        ++executions;
        return {true, "inspected", {}};
    }
    int executions = 0;
private:
    hakui::EntityId id_;
};

void same_actor_target_request_is_not_reentered_from_observer() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<CountingTarget>(501);
    assert(service.registerTarget(target));
    const hakui::InteractionRequest request{11, 501, hakui::InteractionVerb::Inspect, {}};
    bool triggered = false;
    bool nestedHandled = true;
    std::vector<std::string> topics;
    bus.subscribe([&](const spiral::Signal& signal) {
        topics.push_back(signal.topic);
        if (signal.topic == "interaction.request" && !triggered) {
            triggered = true;
            nestedHandled = service.interact(request).handled;
        }
    });
    assert(service.interact(request).handled);
    assert(triggered);
    assert(!nestedHandled);
    assert(target->executions == 1);
    assert(std::count(topics.begin(), topics.end(), "interaction.request") == 1);
    assert(std::count(topics.begin(), topics.end(), "interaction.exec") == 1);
    // A subsequent independent interaction must not be blocked.
    assert(service.interact(request).handled);
    assert(target->executions == 2);
}

void independent_target_is_allowed_during_observer_dispatch() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto first = std::make_shared<CountingTarget>(601);
    auto second = std::make_shared<CountingTarget>(602);
    assert(service.registerTarget(first));
    assert(service.registerTarget(second));
    const hakui::InteractionRequest firstRequest{11, 601, hakui::InteractionVerb::Inspect, {}};
    const hakui::InteractionRequest secondRequest{11, 602, hakui::InteractionVerb::Inspect, {}};
    bool nestedHandled = false;
    bool triggered = false;
    bus.subscribe([&](const spiral::Signal& signal) {
        if (signal.topic == "interaction.request" &&
            signal.payload.find("target=601") != std::string::npos && !triggered) {
            triggered = true;
            nestedHandled = service.interact(secondRequest).handled;
        }
    });
    assert(service.interact(firstRequest).handled);
    assert(triggered && nestedHandled);
    assert(first->executions == 1 && second->executions == 1);
}

void exception_during_observer_delivery_releases_guard() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<CountingTarget>(701);
    assert(service.registerTarget(target));
    const hakui::InteractionRequest request{11, 701, hakui::InteractionVerb::Inspect, {}};
    const auto listenerId = bus.subscribe([](const spiral::Signal& signal) {
        if (signal.topic == "interaction.request") throw std::runtime_error("observer fault");
    });
    bool caught = false;
    try { (void)service.interact(request); }
    catch (const std::runtime_error&) { caught = true; }
    assert(caught);
    assert(target->executions == 0);
    bus.unsubscribe(listenerId);
    assert(service.interact(request).handled);
    assert(target->executions == 1);
}
} // namespace

int main() {
    same_actor_target_request_is_not_reentered_from_observer();
    independent_target_is_allowed_during_observer_dispatch();
    exception_during_observer_delivery_releases_guard();
}
