#include "interaction/InteractionService.hpp"

#include <string>
#include <utility>

namespace hakui {

namespace {

std::string interactionPayload(const InteractionRequest& request)
{
    return std::string(interactionVerbName(request.verb)) +
           "|actor=" + std::to_string(request.actor) +
           "|target=" + std::to_string(request.target);
}

} // namespace

InteractionService::InteractionService(spiral::RouterBus& bus)
    : bus_(bus)
{
}

bool InteractionService::registerTarget(const std::shared_ptr<Interactable>& target)
{
    if (!target) {
        return false;
    }

    // Validate the exact identity used as the registration key. Calling a
    // virtual getter twice may yield different IDs or trigger side effects.
    const EntityId id = target->interactionId();
    if (id == 0) {
        return false;
    }
    const auto it = targets_.find(id);
    if (it != targets_.end()) {
        if (!it->second.target.expired()) {
            return false;
        }
        targets_.erase(it);
    }

    targets_[id] = {target, nextRegistrationToken_++};
    return true;
}

bool InteractionService::unregisterTarget(EntityId id)
{
    return targets_.erase(id) > 0;
}

void InteractionService::pruneExpired()
{
    for (auto it = targets_.begin(); it != targets_.end();) {
        if (it->second.target.expired()) {
            it = targets_.erase(it);
        } else {
            ++it;
        }
    }
}

std::size_t InteractionService::targetCount()
{
    pruneExpired();
    return targets_.size();
}

std::vector<InteractionOption> InteractionService::options(EntityId actor, EntityId target)
{
    // A zero actor has no authority to query capabilities or trigger callbacks.
    // Keep menu discovery consistent with interact()'s invalid_actor gate.
    if (actor == 0) {
        return {};
    }

    // A target may synchronously ask for its own menu while computing it.
    // Reject only identical in-flight lookups; unrelated actors/targets work.
    const auto key = std::make_pair(actor, target);
    if (!activeOptionLookups_.insert(key).second) {
        return {};
    }
    struct LookupCleanup {
        std::set<std::pair<EntityId, EntityId>>& active;
        std::pair<EntityId, EntityId> key;
        ~LookupCleanup() { active.erase(key); }
    } cleanup{activeOptionLookups_, key};

    const auto object = findTarget(target);
    if (!object) {
        return {};
    }

    // A synchronous options callback may unregister or replace its target.
    // Never expose options from a registration that is no longer current.
    // Identity tokens also reject unregister/re-register of the same object.
    const std::uint64_t registrationToken = targets_.at(target).token;
    auto offered = object->interactionOptions(actor);
    const auto current = targets_.find(target);
    if (current == targets_.end() || current->second.token != registrationToken ||
        current->second.target.lock() != object) {
        return {};
    }
    return offered;
}

InteractionResult InteractionService::interact(const InteractionRequest& request)
{
    const auto key = std::make_pair(request.actor, request.target);
    if (!activeInteractions_.insert(key).second) {
        // Router observers are synchronous. Do not emit another request/error
        // for a recursive duplicate: an error observer could recurse too.
        return {};
    }
    struct InFlightCleanup {
        std::set<std::pair<EntityId, EntityId>>& active;
        std::pair<EntityId, EntityId> key;
        ~InFlightCleanup() { active.erase(key); }
    } cleanup{activeInteractions_, key};

    emitRequest(request);

    if (request.actor == 0) {
        emitError(request, "interaction.invalid_actor", "actor id must be nonzero");
        return {};
    }

    const auto object = findTarget(request.target);
    if (!object) {
        emitError(request, "interaction.target_missing", "target is not registered");
        return {};
    }

    // The options callback may synchronously revoke or replace its own
    // registration. Check the token as well as the object to prevent ABA
    // re-registration of the same shared_ptr from authorizing stale work.
    const std::uint64_t registrationToken = targets_.at(request.target).token;
    const auto offered = object->interactionOptions(request.actor);
    const auto current = targets_.find(request.target);
    if (current == targets_.end() || current->second.token != registrationToken ||
        current->second.target.lock() != object) {
        emitError(request, "interaction.target_changed",
                  "target registration changed during options lookup");
        return {};
    }
    if (!offersVerb(offered, request.verb)) {
        emitError(request, "interaction.denied", "requested verb is not currently offered");
        return {};
    }

    InteractionResult result = object->interact(request);
    if (!result.handled) {
        emitError(request, "interaction.unhandled", "target declined interaction");
        return result;
    }

    emitResult(request, result);
    return result;
}

std::shared_ptr<Interactable> InteractionService::findTarget(EntityId id)
{
    if (id == 0) {
        return {};
    }

    const auto it = targets_.find(id);
    if (it == targets_.end()) {
        return {};
    }

    auto target = it->second.target.lock();
    if (!target) {
        targets_.erase(it);
    }
    return target;
}

bool InteractionService::offersVerb(
    const std::vector<InteractionOption>& options,
    InteractionVerb verb
) const
{
    for (const InteractionOption& option : options) {
        if (option.verb == verb) {
            return true;
        }
    }
    return false;
}

void InteractionService::emitRequest(const InteractionRequest& request)
{
    spiral::Signal signal;
    signal.kind = spiral::SignalKind::CommandIn;
    signal.source = "hakui.interaction";
    signal.destination = "spiral.core";
    signal.topic = "interaction.request";
    signal.payload = interactionPayload(request);
    bus_.emit(std::move(signal));
}

void InteractionService::emitResult(
    const InteractionRequest& request,
    const InteractionResult& result
)
{
    spiral::Signal executed;
    executed.kind = spiral::SignalKind::Exec;
    executed.source = "hakui.interaction";
    executed.destination = "spiral.core";
    executed.topic = "interaction.exec";
    executed.payload = interactionPayload(request);
    if (!result.output.empty()) {
        executed.payload += "|output=" + result.output;
    }
    bus_.emit(std::move(executed));

    if (!result.statePatch.empty()) {
        spiral::Signal state;
        state.kind = spiral::SignalKind::State;
        state.source = "hakui.interaction";
        state.destination = "spiral.core";
        state.topic = "interaction.state";
        state.payload = interactionPayload(request);
        state.statePatch = result.statePatch;
        bus_.emit(std::move(state));
    }
}

void InteractionService::emitError(
    const InteractionRequest& request,
    const char* topic,
    const char* message
)
{
    spiral::Signal error;
    error.kind = spiral::SignalKind::Error;
    error.source = "hakui.interaction";
    error.destination = "spiral.core";
    error.topic = topic;
    error.payload = interactionPayload(request) + "|error=" + message;
    bus_.emit(std::move(error));
}

} // namespace hakui
