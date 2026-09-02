#include "spiral/hakui/SpiralCortexClient.hpp"

#include <cassert>
#include <string>

int main()
{
    hakui::GameRuntime runtime;
    runtime.resetSession(123.0f);
    runtime.advanceWorld(2.0f);
    runtime.player().displayName = "L9 TEST";
    runtime.player().x = 4.25f;
    runtime.player().y = 0.0f;
    runtime.player().z = -1.5f;
    runtime.player().health = 87.0f;
    runtime.player().stamina = 61.0f;

    const hakui::HakuiSnapshot snapshot = hakui::captureHakuiSnapshot(runtime);
    const std::string context =
        hakui::SpiralCortexClient::buildReadOnlyContext(snapshot);

    // L9 invariant 1: cortex context is a frozen observation packet, not a
    // command surface. The authority policy is explicit in every request.
    assert(context.find("SPIRAL_HAKUI_CONTEXT/1") != std::string::npos);
    assert(context.find("authority=read_only") != std::string::npos);
    assert(context.find("policy.no_world_mutation=true") != std::string::npos);
    assert(context.find("policy.no_player_control=true") != std::string::npos);
    assert(context.find("policy.no_interaction_execution=true") != std::string::npos);

    // L9 invariant 2: the actual HAKUI world/player truth is serialized for the
    // Spiral Ether AI host rather than being reconstructed inside the AI.
    assert(context.find(snapshot.world.worldId) != std::string::npos);
    assert(context.find("L9 TEST") != std::string::npos);
    assert(context.find("player.health=87.000") != std::string::npos);
    assert(context.find("player.stamina=61.000") != std::string::npos);
    assert(context.find("player.position=4.250,0.000,-1.500") != std::string::npos);
    assert(context.find("SPIRAL PRESENCE NODE") != std::string::npos);

    // L9 invariant 3: transport defaults to a loopback-only fixed port. A
    // missing server is an ordinary unbound state, not a HAKUI boot failure.
    const hakui::SpiralCortexClient client;
    assert(client.endpoint().port == hakui::SpiralCortexClient::defaultPort);
    assert(hakui::SpiralCortexClient::protocolName == "SPIRAL_HAKUI/1");

    // Empty prompts are rejected before any socket transaction is attempted.
    const hakui::SpiralCortexReply empty = client.ask(snapshot, {});
    assert(!empty.connected);
    assert(!empty.ok);
    assert(empty.error.find("empty") != std::string::npos);

    // Context generation itself must not mutate authoritative HAKUI state.
    const hakui::HakuiSnapshot after = hakui::captureHakuiSnapshot(runtime);
    assert(after.world.simulationStep == snapshot.world.simulationStep);
    assert(after.player.x == snapshot.player.x);
    assert(after.player.health == snapshot.player.health);
    assert(after.interactions.liveTargetIds == snapshot.interactions.liveTargetIds);

    return 0;
}
