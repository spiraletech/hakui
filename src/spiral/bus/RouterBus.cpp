#include "spiral/bus/RouterBus.hpp"

#include <memory>
#include <utility>
#include <vector>

namespace spiral {

RouterBus::ListenerId RouterBus::subscribe(Listener listener)
{
    const ListenerId id = nextListenerId_++;
    listeners_.emplace(id, std::make_shared<Listener>(std::move(listener)));
    return id;
}

void RouterBus::unsubscribe(ListenerId id)
{
    listeners_.erase(id);
}

SignalId RouterBus::emit(Signal signal)
{
    signal.id = nextSignalId_++;

    // Snapshot listeners in ascending subscription-ID order. This makes
    // observer delivery reproducible without granting policy authority.
    // A listener may unsubscribe itself or
    // another listener while handling an event; that must not invalidate the
    // active iteration over the live registry.
    std::vector<std::shared_ptr<Listener>> snapshot;
    snapshot.reserve(listeners_.size());

    for (const auto& [id, listener] : listeners_) {
        (void)id;
        if (listener && *listener) {
            snapshot.push_back(listener);
        }
    }

    for (const auto& listener : snapshot) {
        (*listener)(signal);
    }

    return signal.id;
}

std::size_t RouterBus::listenerCount() const noexcept
{
    return listeners_.size();
}

SignalId RouterBus::nextSignalId() const noexcept
{
    return nextSignalId_;
}

} // namespace spiral
