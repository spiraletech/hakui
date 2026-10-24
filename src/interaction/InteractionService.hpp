#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <set>
#include <utility>
#include <unordered_map>
#include <vector>

#include "interaction/Interactable.hpp"
#include "spiral/bus/RouterBus.hpp"

namespace hakui {

// Non-owning interaction registry + execution bridge.
// World/entity ownership remains elsewhere; weak_ptr registration means an
// expired object cannot become a dangling interaction target.
class InteractionService {
public:
    explicit InteractionService(spiral::RouterBus& bus);

    bool registerTarget(const std::shared_ptr<Interactable>& target);
    bool unregisterTarget(EntityId id);
    void pruneExpired();

    std::size_t targetCount();
    std::vector<InteractionOption> options(EntityId actor, EntityId target);
    InteractionResult interact(const InteractionRequest& request);

private:
    std::shared_ptr<Interactable> findTarget(EntityId id);
    bool offersVerb(
        const std::vector<InteractionOption>& options,
        InteractionVerb verb
    ) const;

    void emitRequest(const InteractionRequest& request);
    void emitResult(const InteractionRequest& request, const InteractionResult& result);
    void emitError(const InteractionRequest& request, const char* topic, const char* message);

private:
    spiral::RouterBus& bus_;
    // Every successful registration receives a fresh identity token. A target
    // may unregister/re-register itself during a synchronous options callback.
    struct Registration {
        std::weak_ptr<Interactable> target;
        std::uint64_t token = 0;
    };
    std::unordered_map<EntityId, Registration> targets_;
    std::uint64_t nextRegistrationToken_ = 1;
    // Per actor/target in-flight guard: observers cannot recursively dispatch
    // the same interaction while the original request is being handled.
    std::set<std::pair<EntityId, EntityId>> activeInteractions_;
    // Suppress same actor/target menu recursion without blocking independent lookups.
    std::set<std::pair<EntityId, EntityId>> activeOptionLookups_;
};

} // namespace hakui
