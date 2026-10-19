#include <cassert>
#include <string>
#include <vector>

#include "spiral/bus/RouterBus.hpp"

namespace {

void observers_are_called_in_subscription_order()
{
    spiral::RouterBus bus;
    std::vector<int> seen;
    const auto first = bus.subscribe([&](const spiral::Signal&) { seen.push_back(1); });
    const auto second = bus.subscribe([&](const spiral::Signal&) { seen.push_back(2); });
    bus.subscribe([&](const spiral::Signal&) { seen.push_back(3); });
    assert(first < second);
    bus.emit({});
    assert((seen == std::vector<int>{1, 2, 3}));
}

void mutable_observer_state_survives_successive_emissions()
{
    spiral::RouterBus bus;
    std::vector<int> seen;
    const auto id = bus.subscribe([count = 0, &seen](const spiral::Signal&) mutable {
        seen.push_back(++count);
    });
    bus.emit({});
    bus.emit({});
    bus.emit({});
    assert((seen == std::vector<int>{1, 2, 3}));
    bus.unsubscribe(id);
    bus.emit({});
    assert((seen == std::vector<int>{1, 2, 3}));
}

void unsubscribe_during_dispatch_preserves_only_current_snapshot()
{
    spiral::RouterBus bus;
    std::vector<int> seen;
    spiral::RouterBus::ListenerId second = 0;
    bus.subscribe([&](const spiral::Signal&) {
        seen.push_back(1);
        bus.unsubscribe(second);
    });
    second = bus.subscribe([count = 0, &seen](const spiral::Signal&) mutable {
        seen.push_back(++count * 10);
    });
    bus.emit({});
    bus.emit({});
    assert((seen == std::vector<int>{1, 10, 1}));
}

void nested_emission_has_independent_snapshot_and_shared_observer_identity()
{
    spiral::RouterBus bus;
    std::vector<std::string> seen;
    bus.subscribe([&](const spiral::Signal& signal) {
        if (signal.topic == "outer") {
            spiral::Signal inner;
            inner.topic = "inner";
            bus.emit(inner);
        }
    });
    bus.subscribe([count = 0, &seen](const spiral::Signal& signal) mutable {
        seen.push_back(signal.topic + ":" + std::to_string(++count));
    });
    spiral::Signal outer;
    outer.topic = "outer";
    bus.emit(outer);
    spiral::Signal later;
    later.topic = "later";
    bus.emit(later);
    assert((seen == std::vector<std::string>{"inner:1", "outer:2", "later:3"}));
}

} // namespace

int main()
{
    observers_are_called_in_subscription_order();
    mutable_observer_state_survives_successive_emissions();
    unsubscribe_during_dispatch_preserves_only_current_snapshot();
    nested_emission_has_independent_snapshot_and_shared_observer_identity();
}
