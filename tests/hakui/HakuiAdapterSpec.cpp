#include "spiral/hakui/HakuiAdapter.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <memory>
#include <type_traits>
#include <vector>

namespace {

bool near(float left, float right, float epsilon = 0.0001f) noexcept
{
    return std::fabs(left - right) <= epsilon;
}

class AdapterTarget final : public hakui::Interactable {
public:
    explicit AdapterTarget(hakui::EntityId id) : id_(id) {}

    hakui::EntityId interactionId() const noexcept override
    {
        return id_;
    }

    std::vector<hakui::InteractionOption> interactionOptions(
        hakui::EntityId actor
    ) const override
    {
        (void)actor;
        return {{hakui::InteractionVerb::Inspect, "Inspect adapter target"}};
    }

    hakui::InteractionResult interact(
        const hakui::InteractionRequest& request
    ) override
    {
        hakui::InteractionResult result;
        result.handled = request.verb == hakui::InteractionVerb::Inspect;
        return result;
    }

private:
    hakui::EntityId id_ = 0;
};

} // namespace

int main()
{
    static_assert(std::is_constructible_v<hakui::HakuiAdapter, const hakui::GameRuntime&>);
    static_assert(!std::is_constructible_v<hakui::HakuiAdapter, hakui::GameRuntime&&>);
    static_assert(!std::is_constructible_v<hakui::HakuiAdapter, const hakui::GameRuntime&&>);

    hakui::GameRuntime runtime;
    runtime.resetSession(375.0f);
    runtime.advanceWorld(4.25f);

    runtime.player().displayName = "L7 TEST";
    runtime.player().health = 88.0f;
    runtime.player().stamina = 62.0f;

    auto targetHigh = std::make_shared<AdapterTarget>(7302);
    auto targetLow = std::make_shared<AdapterTarget>(7301);
    assert(runtime.interactionRegistry().registerTarget(targetHigh));
    assert(runtime.interactionRegistry().registerTarget(targetLow));

    const hakui::GameRuntime& readOnlyRuntime = runtime;
    const hakui::HakuiAdapter adapter(readOnlyRuntime);

    // L7 invariant 1: stable command catalog is explicit and read-only.
    assert(adapter.supports(hakui::HakuiAdapter::inspectWorldCommand));
    assert(adapter.supports(hakui::HakuiAdapter::inspectPlayerCommand));
    assert(adapter.supports(hakui::HakuiAdapter::inspectNearbyCommand));
    assert(adapter.supports(hakui::HakuiAdapter::inspectTimeCommand));
    assert(adapter.supports(hakui::HakuiAdapter::inspectInteractionsCommand));
    assert(!adapter.supports("hakui.inspect_weather"));
    assert(!adapter.supports("hakui.set_player_position"));

    const hakui::HakuiSnapshot before = adapter.snapshot();

    const hakui::HakuiWorldSnapshot world = adapter.inspectWorld();
    assert(world.worldId == hakui::HakuiWorldState::canonicalWorldId);
    assert(world.scene == hakui::HakuiWorldState::canonicalScene);
    assert(near(world.elapsedSeconds, 4.25f));
    assert(world.simulationStep == 1);
    assert(!world.affordances.empty());
    assert(!world.seats.empty());

    const hakui::HakuiPlayerSnapshot player = adapter.inspectPlayer();
    assert(player.displayName == "L7 TEST");
    assert(near(player.health, 88.0f));
    assert(near(player.stamina, 62.0f));
    assert(near(player.money, 375.0f));

    const hakui::HakuiTimeInspection time = adapter.inspectTime();
    assert(near(time.elapsedSeconds, 4.25f));
    assert(time.simulationStep == 1);

    const hakui::HakuiInteractionSnapshot interactions = adapter.inspectInteractions();
    assert(interactions.liveTargetIds.size() == 2);
    assert(interactions.liveTargetIds[0] == 7301);
    assert(interactions.liveTargetIds[1] == 7302);

    // L7 invariant 2: nearby inspection derives semantic proximity from the
    // copied affordance contract, not renderer primitives.
    const hakui::WorldAffordanceVolume* couch = runtime.blackRoom().affordanceById(1002);
    assert(couch != nullptr);
    runtime.player().x = couch->primaryAnchor.x;
    runtime.player().y = couch->primaryAnchor.y;
    runtime.player().z = couch->primaryAnchor.z;

    const hakui::HakuiNearbyInspection nearby = adapter.inspectNearby(1.0f);
    assert(near(nearby.radius, 1.0f));
    bool foundCouch = false;
    for (const auto& object : nearby.objects) {
        if (object.affordanceId != 1002) {
            continue;
        }
        foundCouch = true;
        assert(object.distance <= 1.0f);
        assert(object.seatCount == 2);
        assert(object.occupiedSeatCount == 0);
        assert(hakui::hasAffordance(object.affordances, hakui::WorldAffordance::Seat));
    }
    assert(foundCouch);

    // L7 invariant 3: generic tool dispatch returns typed payloads and rejects
    // unsupported/future capabilities rather than inventing world truth.
    const auto worldResult = adapter.execute("hakui.inspect_world");
    assert(worldResult);
    assert(worldResult.status == hakui::HakuiAdapterStatus::Ok);
    assert(std::get_if<hakui::HakuiWorldSnapshot>(&worldResult.payload) != nullptr);

    const auto nearbyResult = adapter.execute("hakui.inspect_nearby", 2.0f);
    assert(nearbyResult);
    const auto* nearbyPayload = std::get_if<hakui::HakuiNearbyInspection>(&nearbyResult.payload);
    assert(nearbyPayload != nullptr);
    assert(near(nearbyPayload->radius, 2.0f));

    const auto invalidNearby = adapter.execute(
        "hakui.inspect_nearby",
        std::numeric_limits<float>::quiet_NaN()
    );
    assert(!invalidNearby);
    assert(invalidNearby.status == hakui::HakuiAdapterStatus::InvalidArgument);
    assert(std::holds_alternative<std::monostate>(invalidNearby.payload));

    const auto weather = adapter.execute("hakui.inspect_weather");
    assert(!weather);
    assert(weather.status == hakui::HakuiAdapterStatus::UnsupportedCommand);
    assert(std::holds_alternative<std::monostate>(weather.payload));

    // L7 invariant 4: observation cannot mutate simulation authority.
    const hakui::HakuiSnapshot observationStart = adapter.snapshot();
    (void)adapter.inspectWorld();
    (void)adapter.inspectPlayer();
    (void)adapter.inspectNearby(8.0f);
    (void)adapter.inspectTime();
    (void)adapter.inspectInteractions();
    (void)adapter.execute("hakui.inspect_world");
    (void)adapter.execute("hakui.inspect_player");
    (void)adapter.execute("hakui.inspect_nearby", 4.0f);
    (void)adapter.execute("hakui.inspect_time");
    (void)adapter.execute("hakui.inspect_interactions");
    const hakui::HakuiSnapshot observationEnd = adapter.snapshot();

    assert(observationStart.world.simulationStep == observationEnd.world.simulationStep);
    assert(near(observationStart.world.elapsedSeconds, observationEnd.world.elapsedSeconds));
    assert(observationStart.world.occupiedSeatCount == observationEnd.world.occupiedSeatCount);
    assert(observationStart.player.displayName == observationEnd.player.displayName);
    assert(near(observationStart.player.x, observationEnd.player.x));
    assert(near(observationStart.player.y, observationEnd.player.y));
    assert(near(observationStart.player.z, observationEnd.player.z));
    assert(near(observationStart.player.health, observationEnd.player.health));
    assert(near(observationStart.player.money, observationEnd.player.money));
    assert(observationStart.interactions.liveTargetIds == observationEnd.interactions.liveTargetIds);

    // Earlier packets remain frozen after later runtime mutation.
    runtime.player().health = 11.0f;
    runtime.advanceWorld(0.5f);
    const auto afterMutation = adapter.inspectPlayer();
    assert(near(afterMutation.health, 11.0f));
    assert(near(player.health, 88.0f));
    assert(near(before.world.elapsedSeconds, 4.25f));

    return 0;
}
