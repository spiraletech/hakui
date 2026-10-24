#include <cassert>
#include <cstdint>
#include <memory>
#include <vector>

#include "interaction/InteractionService.hpp"

namespace {

class CountingMenuTarget final : public hakui::Interactable {
public:
    explicit CountingMenuTarget(hakui::EntityId id) : id_(id) {}
    hakui::EntityId interactionId() const noexcept override { return id_; }
    std::vector<hakui::InteractionOption> interactionOptions(hakui::EntityId actor) const override {
        ++queries;
        lastActor = actor;
        return {{hakui::InteractionVerb::Open, "Open"}};
    }
    hakui::InteractionResult interact(const hakui::InteractionRequest&) override {
        ++executions;
        return {true, "opened", {}};
    }
    mutable unsigned queries = 0;
    mutable hakui::EntityId lastActor = 0;
    unsigned executions = 0;
private:
    hakui::EntityId id_;
};

void zero_actor_cannot_discover_interaction_menu() {
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<CountingMenuTarget>(900);
    assert(service.registerTarget(target));
    unsigned busEvents = 0;
    bus.subscribe([&](const spiral::Signal&) { ++busEvents; });

    assert(service.options(0, 900).empty());
    assert(service.options(0, 0).empty());
    assert(target->queries == 0);
    assert(busEvents == 0);
    assert(service.targetCount() == 1);

    assert(service.options(17, 900).size() == 1);
    assert(target->queries == 1);
    assert(target->lastActor == 17);
    assert(busEvents == 0);

    const hakui::InteractionRequest invalid{0, 900, hakui::InteractionVerb::Open, {}};
    assert(!service.interact(invalid).handled);
    assert(target->queries == 1);
    assert(target->executions == 0);
    assert(busEvents == 2); // request and invalid_actor error only

    const hakui::InteractionRequest valid{17, 900, hakui::InteractionVerb::Open, {}};
    assert(service.interact(valid).handled);
    assert(target->queries == 2);
    assert(target->executions == 1);
    assert(busEvents == 4); // request and exec; no state patch
}

} // namespace

int main() {
    zero_actor_cannot_discover_interaction_menu();
}
