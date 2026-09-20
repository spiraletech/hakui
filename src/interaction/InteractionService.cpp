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

InteractionService::InteractionService(
    InteractionRegistry& registry,
    spiral::RouterBus& bus
)
    : registry_(registry),
      bus_(bus)
{
}

bool InteractionService::registerTarget(const std::shared_ptr<Interactable>& target)
{
    return registry_.registerTarget(target);
}

bool InteractionService::unregisterTarget(EntityId id)
{
    return registry_.unregisterTarget(id);
}

void InteractionService::pruneExpired()
{
    registry_.pruneExpired();
}

std::size_t InteractionService::targetCount()
{
    return registry_.targetCount();
}

std::vector<InteractionOption> InteractionService::options(
    EntityId actor,
    EntityId target
)
{
    return registry_.options(actor, target);
}

InteractionResult InteractionService::interact(const InteractionRequest& request)
{
    emitRequest(request);

    if (request.actor == 0) {
        emitError(request, "interaction.invalid_actor", "actor id must be nonzero");
        return {};
    }

    const auto object = registry_.resolve(request.target);
    if (!object) {
        emitError(request, "interaction.target_missing", "target is not registered");
        return {};
    }

    const auto offered = object->interactionOptions(request.actor);
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
