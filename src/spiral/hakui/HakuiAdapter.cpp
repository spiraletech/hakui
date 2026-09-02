#include "spiral/hakui/HakuiAdapter.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace hakui {
namespace {

float axisDistance(float value, float minimum, float maximum) noexcept
{
    if (value < minimum) {
        return minimum - value;
    }
    if (value > maximum) {
        return value - maximum;
    }
    return 0.0f;
}

float distanceToAffordance(
    const HakuiPlayerSnapshot& player,
    const HakuiAffordanceSnapshot& affordance
) noexcept
{
    const float dx = axisDistance(player.x, affordance.minimumX, affordance.maximumX);
    const float dy = axisDistance(player.y, affordance.minimumY, affordance.maximumY);
    const float dz = axisDistance(player.z, affordance.minimumZ, affordance.maximumZ);
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

HakuiNearbyInspection deriveNearby(const HakuiSnapshot& snapshot, float radius)
{
    HakuiNearbyInspection nearby;
    nearby.radius = radius;

    for (const HakuiAffordanceSnapshot& affordance : snapshot.world.affordances) {
        const float distance = distanceToAffordance(snapshot.player, affordance);
        if (distance > radius) {
            continue;
        }

        HakuiNearbyObservation observation;
        observation.affordanceId = affordance.id;
        observation.label = affordance.label;
        observation.affordances = affordance.affordances;
        observation.distance = distance;
        observation.primaryAnchor = affordance.primaryAnchor;

        for (const HakuiSeatSnapshot& seat : snapshot.world.seats) {
            if (seat.furnitureAffordanceId != affordance.id) {
                continue;
            }
            ++observation.seatCount;
            if (seat.occupied) {
                ++observation.occupiedSeatCount;
            }
        }

        nearby.objects.push_back(std::move(observation));
    }

    std::sort(
        nearby.objects.begin(),
        nearby.objects.end(),
        [](const HakuiNearbyObservation& left, const HakuiNearbyObservation& right) {
            if (left.distance != right.distance) {
                return left.distance < right.distance;
            }
            return left.affordanceId < right.affordanceId;
        }
    );

    return nearby;
}

} // namespace

HakuiSnapshot HakuiAdapter::snapshot() const
{
    return captureHakuiSnapshot(runtime_);
}

HakuiWorldSnapshot HakuiAdapter::inspectWorld() const
{
    return snapshot().world;
}

HakuiPlayerSnapshot HakuiAdapter::inspectPlayer() const
{
    return snapshot().player;
}

HakuiNearbyInspection HakuiAdapter::inspectNearby(float radius) const
{
    if (!std::isfinite(radius) || radius < 0.0f) {
        return {};
    }
    return deriveNearby(snapshot(), radius);
}

HakuiTimeInspection HakuiAdapter::inspectTime() const
{
    const HakuiSnapshot current = snapshot();
    return {current.world.elapsedSeconds, current.world.simulationStep};
}

HakuiInteractionSnapshot HakuiAdapter::inspectInteractions() const
{
    return snapshot().interactions;
}

bool HakuiAdapter::supports(std::string_view command) const noexcept
{
    return std::find(readOnlyCommands.begin(), readOnlyCommands.end(), command) !=
           readOnlyCommands.end();
}

HakuiAdapterResult HakuiAdapter::execute(
    std::string_view command,
    float nearbyRadius
) const
{
    HakuiAdapterResult result;
    result.command = std::string(command);

    if (!supports(command)) {
        result.status = HakuiAdapterStatus::UnsupportedCommand;
        result.message = "HAKUI does not expose this read-only inspection command";
        return result;
    }

    if (command == inspectNearbyCommand &&
        (!std::isfinite(nearbyRadius) || nearbyRadius < 0.0f)) {
        result.status = HakuiAdapterStatus::InvalidArgument;
        result.message = "nearby radius must be finite and non-negative";
        return result;
    }

    const HakuiSnapshot current = snapshot();
    result.snapshotVersion = current.version;

    if (command == inspectWorldCommand) {
        result.payload = current.world;
    } else if (command == inspectPlayerCommand) {
        result.payload = current.player;
    } else if (command == inspectNearbyCommand) {
        result.payload = deriveNearby(current, nearbyRadius);
    } else if (command == inspectTimeCommand) {
        result.payload = HakuiTimeInspection{
            current.world.elapsedSeconds,
            current.world.simulationStep
        };
    } else if (command == inspectInteractionsCommand) {
        result.payload = current.interactions;
    }

    result.status = HakuiAdapterStatus::Ok;
    return result;
}

} // namespace hakui
