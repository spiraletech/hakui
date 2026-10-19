#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "spiral/core/SpiralTypes.hpp"

namespace spiral {

class RouterBus {
public:
    using ListenerId = std::uint64_t;
    using Listener = std::function<void(const Signal&)>;

    ListenerId subscribe(Listener listener);
    void unsubscribe(ListenerId id);

    // Plumbing only: sequence and distribute the packet.
    // No policy, no action selection, no AUM interpretation.
    // Event identity belongs exclusively to this bus: every emission receives
    // a fresh monotonically increasing id regardless of caller-provided data.
    SignalId emit(Signal signal);

    std::size_t listenerCount() const noexcept;
    SignalId nextSignalId() const noexcept;

private:
    ListenerId nextListenerId_ = 1;
    SignalId nextSignalId_ = 1;
    // Stable ascending subscription order is required for reproducible
    // synchronous observer delivery. Shared callback ownership keeps mutable
    // subscription state intact across emission snapshots while allowing
    // unsubscribe during dispatch without invalidating in-flight listeners.
    // Not an audio-thread-safe or concurrent event bus.
    std::map<ListenerId, std::shared_ptr<Listener>> listeners_;
};

} // namespace spiral
