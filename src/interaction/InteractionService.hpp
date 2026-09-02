#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include "interaction/InteractionRegistry.hpp"
#include "spiral/bus/RouterBus.hpp"

namespace hakui {

// L5 interaction execution/telemetry bridge.
//
// Target membership is authoritative in InteractionRegistry, which is owned by
// GameRuntime. InteractionService only resolves/executes requests and reports
// their typed lifecycle over RouterBus into Spiral Core.
class InteractionService {
public:
    InteractionService(InteractionRegistry& registry, spiral::RouterBus& bus);

    // Compatibility surface for existing callers. Membership mutations are
    // delegated to the externally owned registry rather than stored here.
    bool registerTarget(const std::shared_ptr<Interactable>& target);
    bool unregisterTarget(EntityId id);
    void pruneExpired();

    std::size_t targetCount();
    std::vector<InteractionOption> options(EntityId actor, EntityId target);
    InteractionResult interact(const InteractionRequest& request);

    InteractionRegistry& registry() noexcept { return registry_; }
    const InteractionRegistry& registry() const noexcept { return registry_; }

private:
    bool offersVerb(
        const std::vector<InteractionOption>& options,
        InteractionVerb verb
    ) const;

    void emitRequest(const InteractionRequest& request);
    void emitResult(const InteractionRequest& request, const InteractionResult& result);
    void emitError(const InteractionRequest& request, const char* topic, const char* message);

private:
    InteractionRegistry& registry_;
    spiral::RouterBus& bus_;
};

} // namespace hakui
