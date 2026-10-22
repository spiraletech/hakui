#include <cassert>
#include <memory>
#include <vector>

#include "interaction/InteractionService.hpp"

namespace {

class SequencedIdentityTarget final : public hakui::Interactable {
public:
    SequencedIdentityTarget(hakui::EntityId first, hakui::EntityId later)
        : first_(first), later_(later) {}

    hakui::EntityId interactionId() const noexcept override
    {
        return ++identityCalls == 1 ? first_ : later_;
    }

    std::vector<hakui::InteractionOption> interactionOptions(hakui::EntityId) const override
    {
        return {{hakui::InteractionVerb::Inspect, "Inspect"}};
    }

    hakui::InteractionResult interact(const hakui::InteractionRequest&) override
    {
        return {true, "inspected", {}};
    }

    mutable unsigned identityCalls = 0;

private:
    hakui::EntityId first_;
    hakui::EntityId later_;
};

void accepted_identity_is_read_exactly_once()
{
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<SequencedIdentityTarget>(101, 0);
    assert(service.registerTarget(target));
    assert(target->identityCalls == 1);
    assert(service.targetCount() == 1);
    assert(service.options(7, 101).size() == 1);
    assert(service.options(7, 0).empty());
}

void zero_identity_is_rejected_without_registration()
{
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto target = std::make_shared<SequencedIdentityTarget>(0, 101);
    assert(!service.registerTarget(target));
    assert(target->identityCalls == 1);
    assert(service.targetCount() == 0);
}

void duplicate_registration_remains_rejected()
{
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    auto first = std::make_shared<SequencedIdentityTarget>(102, 0);
    auto second = std::make_shared<SequencedIdentityTarget>(102, 999);
    assert(service.registerTarget(first));
    assert(!service.registerTarget(second));
    assert(first->identityCalls == 1);
    assert(second->identityCalls == 1);
    assert(service.targetCount() == 1);
    assert(service.options(7, 102).size() == 1);
}

void null_target_is_rejected()
{
    spiral::RouterBus bus;
    hakui::InteractionService service(bus);
    assert(!service.registerTarget(nullptr));
    assert(service.targetCount() == 0);
}

} // namespace

int main()
{
    accepted_identity_is_read_exactly_once();
    zero_identity_is_rejected_without_registration();
    duplicate_registration_remains_rejected();
    null_target_is_rejected();
}
