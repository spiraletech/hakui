#pragma once

#include <cstddef>
#include <memory>
#include <unordered_map>
#include <vector>

#include "interaction/Interactable.hpp"

namespace hakui {

// L5 authoritative interaction-target registry.
//
// The registry owns target membership, not world-object lifetime. Targets are
// weakly referenced so world/entity owners remain authoritative and expired
// objects cannot become dangling interaction endpoints.
class InteractionRegistry final {
public:
    InteractionRegistry() = default;
    InteractionRegistry(const InteractionRegistry&) = delete;
    InteractionRegistry& operator=(const InteractionRegistry&) = delete;
    InteractionRegistry(InteractionRegistry&&) = delete;
    InteractionRegistry& operator=(InteractionRegistry&&) = delete;

    bool registerTarget(const std::shared_ptr<Interactable>& target)
    {
        if (!target || target->interactionId() == 0) {
            return false;
        }

        const EntityId id = target->interactionId();
        const auto it = targets_.find(id);
        if (it != targets_.end()) {
            if (!it->second.expired()) {
                return false;
            }
            targets_.erase(it);
        }

        targets_[id] = target;
        return true;
    }

    bool unregisterTarget(EntityId id)
    {
        return targets_.erase(id) > 0;
    }

    void pruneExpired()
    {
        for (auto it = targets_.begin(); it != targets_.end();) {
            if (it->second.expired()) {
                it = targets_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void clear() noexcept
    {
        targets_.clear();
    }

    std::size_t targetCount()
    {
        pruneExpired();
        return targets_.size();
    }

    std::shared_ptr<Interactable> resolve(EntityId id)
    {
        if (id == 0) {
            return {};
        }

        const auto it = targets_.find(id);
        if (it == targets_.end()) {
            return {};
        }

        auto target = it->second.lock();
        if (!target) {
            targets_.erase(it);
        }
        return target;
    }

    std::vector<InteractionOption> options(EntityId actor, EntityId target)
    {
        const auto object = resolve(target);
        if (!object) {
            return {};
        }
        return object->interactionOptions(actor);
    }

private:
    std::unordered_map<EntityId, std::weak_ptr<Interactable>> targets_;
};

} // namespace hakui
