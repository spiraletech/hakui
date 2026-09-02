#include "core/GameRuntime.hpp"

#include <cassert>
#include <cmath>
#include <memory>
#include <vector>

namespace {

bool near(float a, float b, float epsilon = 0.0001f) noexcept
{
    return std::fabs(a - b) <= epsilon;
}

class RuntimeTarget final : public hakui::Interactable {
public:
    explicit RuntimeTarget(hakui::EntityId id) : id_(id) {}

    hakui::EntityId interactionId() const noexcept override { return id_; }

    std::vector<hakui::InteractionOption> interactionOptions(
        hakui::EntityId actor
    ) const override
    {
        (void)actor;
        return {{hakui::InteractionVerb::Inspect, "Inspect runtime target"}};
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
    hakui::GameRuntime runtime;

    // L5: GameRuntime is now a composition root for three explicit authority
    // domains rather than loose player/world/interaction members.
    hakui::HakuiWorldState* const worldAddress = &runtime.world();
    hakui::PlayerRuntime* const playerRuntimeAddress = &runtime.playerRuntime();
    hakui::InteractionRegistry* const interactionAddress =
        &runtime.interactionRegistry();

    assert(&runtime.world() == worldAddress);
    assert(&runtime.playerRuntime() == playerRuntimeAddress);
    assert(&runtime.interactionRegistry() == interactionAddress);

    // Compatibility accessors must resolve into those roots, never copies.
    PlayerState* const playerAddress = &runtime.player();
    assert(playerAddress == &runtime.playerRuntime().state());
    assert(&runtime.movement() == &runtime.playerRuntime().movement());
    assert(&runtime.rideable() == &runtime.playerRuntime().rideable());

    hakui::BlackRoom* const roomAddress = &runtime.world().blackRoom();
    assert(&runtime.blackRoom() == roomAddress);
    assert(&runtime.blackRoom() == &runtime.world().blackRoom());

    const hakui::MovementEnvironment room =
        runtime.blackRoom().movementEnvironment();

    runtime.resetPlayerToSpawn();
    assert(near(runtime.player().x, room.spawnX));
    assert(near(runtime.player().y, room.spawnY));
    assert(near(runtime.player().z, room.spawnZ));
    assert(near(runtime.player().money, 250.0f));
    assert(runtime.player().locomotion == LocomotionMode::OnFoot);

    runtime.player().locomotion = LocomotionMode::Skateboard;
    runtime.player().x = room.spawnX + 4.0f;
    runtime.player().money = 11.0f;
    runtime.resetPlayerToSpawn(99.0f);
    assert(&runtime.player() == playerAddress);
    assert(&runtime.playerRuntime() == playerRuntimeAddress);
    assert(near(runtime.player().x, room.spawnX));
    assert(near(runtime.player().z, room.spawnZ));
    assert(near(runtime.player().money, 99.0f));
    assert(runtime.player().locomotion == LocomotionMode::OnFoot);

    // Interaction membership has its own authority and does not disappear when
    // player or world session state is reset.
    auto target = std::make_shared<RuntimeTarget>(7001);
    assert(runtime.interactionRegistry().registerTarget(target));
    assert(runtime.interactionRegistry().targetCount() == 1);

    // World time advances through the canonical state and stays independent
    // from platform/rendering concerns.
    runtime.advanceWorld(12.5f);
    assert(near(runtime.world().clock().seconds(), 12.5f));
    assert(runtime.world().clock().step() == 1);

    // Full session reset preserves all authority addresses. World + player are
    // restored, while separately owned live interaction membership remains.
    runtime.resetSession(333.0f);
    assert(&runtime.world() == worldAddress);
    assert(&runtime.blackRoom() == roomAddress);
    assert(&runtime.playerRuntime() == playerRuntimeAddress);
    assert(&runtime.player() == playerAddress);
    assert(&runtime.interactionRegistry() == interactionAddress);
    assert(runtime.interactionRegistry().targetCount() == 1);
    assert(runtime.interactionRegistry().resolve(7001) == target);
    assert(near(runtime.world().clock().seconds(), 0.0f));
    assert(runtime.world().clock().step() == 0);
    assert(near(runtime.player().x, room.spawnX));
    assert(near(runtime.player().z, room.spawnZ));
    assert(near(runtime.player().money, 333.0f));

    runtime.interactionRegistry().clear();
    assert(runtime.interactionRegistry().targetCount() == 0);

    return 0;
}
