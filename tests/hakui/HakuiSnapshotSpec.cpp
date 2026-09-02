#include "core/HakuiSnapshot.hpp"

#include <cassert>
#include <cmath>
#include <memory>
#include <type_traits>
#include <vector>

namespace {

bool near(float left, float right, float epsilon = 0.0001f) noexcept
{
    return std::fabs(left - right) <= epsilon;
}

class SnapshotTarget final : public hakui::Interactable {
public:
    explicit SnapshotTarget(hakui::EntityId id) : id_(id) {}

    hakui::EntityId interactionId() const noexcept override
    {
        return id_;
    }

    std::vector<hakui::InteractionOption> interactionOptions(
        hakui::EntityId actor
    ) const override
    {
        (void)actor;
        return {{hakui::InteractionVerb::Inspect, "Inspect snapshot target"}};
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
    static_assert(std::is_copy_constructible_v<hakui::HakuiSnapshot>);
    static_assert(std::is_move_constructible_v<hakui::HakuiSnapshot>);

    hakui::GameRuntime runtime;
    runtime.resetSession(250.0f);

    auto highTarget = std::make_shared<SnapshotTarget>(9002);
    auto lowTarget = std::make_shared<SnapshotTarget>(9001);
    auto expiredTarget = std::make_shared<SnapshotTarget>(9003);
    assert(runtime.interactionRegistry().registerTarget(highTarget));
    assert(runtime.interactionRegistry().registerTarget(lowTarget));
    assert(runtime.interactionRegistry().registerTarget(expiredTarget));
    expiredTarget.reset();

    // L6 invariant 1: capture is accepted through a const runtime and returns
    // only owned value data. Live interaction IDs are deterministic and exclude
    // expired weak targets without requiring registry mutation.
    const hakui::GameRuntime& readOnlyRuntime = runtime;
    const hakui::HakuiSnapshot initial =
        hakui::captureHakuiSnapshot(readOnlyRuntime);

    assert(initial.version == hakui::HakuiSnapshot::schemaVersion);
    assert(initial.world.schemaVersion == hakui::HakuiWorldState::schemaVersion);
    assert(initial.world.worldId == hakui::HakuiWorldState::canonicalWorldId);
    assert(initial.world.scene == hakui::HakuiWorldState::canonicalScene);
    assert(near(initial.world.elapsedSeconds, 0.0f));
    assert(initial.world.simulationStep == 0);
    assert(initial.world.geometryPrimitiveCount == runtime.blackRoom().geometry().size());
    assert(initial.world.affordanceCount == runtime.blackRoom().affordances().size());
    assert(initial.world.seatAnchorCount == runtime.blackRoom().seatAnchors().size());
    assert(initial.world.occupiedSeatCount == 0);
    assert(initial.player.displayName == "ETHER");
    assert(near(initial.player.money, 250.0f));
    assert(initial.interactions.liveTargetIds.size() == 2);
    assert(initial.interactions.liveTargetIds[0] == 9001);
    assert(initial.interactions.liveTargetIds[1] == 9002);

    // L6 invariant 2: runtime truth is copied exactly at capture time.
    runtime.advanceWorld(2.5f);
    runtime.player().displayName = "SNAPSHOT TEST";
    runtime.player().health = 73.0f;
    runtime.player().stamina = 41.0f;
    runtime.player().locomotion = LocomotionMode::OnFoot;

    const hakui::WorldAffordanceVolume* couch =
        runtime.blackRoom().affordanceById(1002);
    assert(couch != nullptr);
    runtime.player().x = couch->secondaryAnchor.x;
    runtime.player().y = couch->secondaryAnchor.y;
    runtime.player().z = couch->secondaryAnchor.z;
    runtime.player().yaw = couch->secondaryAnchor.yaw;
    runtime.player().grounded = true;
    runtime.player().activity = PlayerActivity::Roaming;
    assert(runtime.blackRoom().engageNearest(runtime.player()));

    const hakui::HakuiSnapshot seated = hakui::captureHakuiSnapshot(runtime);
    assert(near(seated.world.elapsedSeconds, 2.5f));
    assert(seated.world.simulationStep == 1);
    assert(seated.world.occupiedSeatCount == 1);
    assert(seated.player.displayName == "SNAPSHOT TEST");
    assert(near(seated.player.health, 73.0f));
    assert(near(seated.player.stamina, 41.0f));
    assert(seated.player.activity == PlayerActivity::CouchSeated);
    assert(seated.player.seatOccupancy);
    assert(seated.player.activeAffordanceId == 1002);
    assert(seated.player.activeSeatAnchorId != 0);

    // L6 invariant 3: a previously captured packet is frozen truth. Later
    // simulation mutations cannot change it through hidden references.
    const float frozenX = seated.player.x;
    const std::uint32_t frozenSeat = seated.player.activeSeatAnchorId;
    assert(runtime.blackRoom().leaveInteraction(runtime.player()));
    runtime.player().x += 5.0f;
    runtime.player().health = 12.0f;
    runtime.advanceWorld(1.0f);

    const hakui::HakuiSnapshot later = hakui::captureHakuiSnapshot(runtime);
    assert(near(seated.player.x, frozenX));
    assert(seated.player.activeSeatAnchorId == frozenSeat);
    assert(seated.player.seatOccupancy);
    assert(seated.world.occupiedSeatCount == 1);
    assert(near(seated.world.elapsedSeconds, 2.5f));

    assert(!later.player.seatOccupancy);
    assert(later.player.activeSeatAnchorId == 0);
    assert(near(later.player.health, 12.0f));
    assert(near(later.world.elapsedSeconds, 3.5f));
    assert(later.world.simulationStep == 2);
    assert(later.world.occupiedSeatCount == 0);

    return 0;
}
