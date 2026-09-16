#include "home/HomeSession.hpp"
#include "core/GameRuntime.hpp"
#include <cassert>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

using namespace hakui;

static PlayerState movedPlayer() {
    GameRuntime runtime;
    runtime.resetPlayerToSpawn();
    const auto environment = runtime.blackRoom().movementEnvironment();
    const float start = runtime.player().z;
    for (int i = 0; i < 20; ++i)
        runtime.movement().update(runtime.player(), MovementInput{0, 1, false, false}, environment, 0.016f);
    assert(std::abs(runtime.player().z - start) > 0.01f);
    return runtime.player();
}
static std::string bytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>{in}, {}};
}
static void sameTransform(const PlayerState& p, const home::Transform& t) {
    assert(HomeBridge::encode(p).value() == t);
}

int main(int argc, char** argv) {
    if (argc == 3) {
        GameRuntime runtime;
        runtime.resetPlayerToSpawn();
        auto opened = HomeSession::open(argv[2], runtime.player());
        assert(opened);
        auto session = std::move(opened).value();
        const auto expected = HomeBridge::encode(movedPlayer()).value();
        if (std::string(argv[1]) == "write") {
            runtime.player() = movedPlayer();
            assert(session->commit(runtime.player()));
            assert(session->world().revision().value() == 2);
            assert(session->save());
        } else {
            assert(std::string(argv[1]) == "read");
            sameTransform(runtime.player(), expected);
            assert(session->entity() == home::EntityId{1});
            assert(session->world().revision().value() == 2);
            // Restart does not manufacture a mutation; unchanged save is exact.
            const auto before = bytes(argv[2]);
            assert(session->commit(runtime.player()));
            assert(session->save());
            assert(bytes(argv[2]) == before);
        }
        std::cout << "HOME process " << argv[1] << " passed\n";
        return 0;
    }

    home::VersionedWorld world{home::WorldId{42}};
    home::EntityCreateInfo info{};
    info.archetype = "test.player";
    info.kind = home::EntityKind::Avatar;
    info.persistent = true;
    info.transform.position = {1234, -2500, 3125};
    info.transform.rotation = {1000, 90000, -2000};
    const auto id = world.create_entity(info).value();
    PlayerState player;
    HomeBridge bridge{world, id};
    assert(!bridge.commit(player)); // Cannot submit before a projection.
    assert(bridge.load(player));
    assert(std::abs(player.x - 1.234f) < 0.00001f);
    assert(std::abs(player.yaw - 1.5707963f) < 0.00001f);
    const auto initial = world.revision();
    assert(bridge.commit(player));
    assert(world.revision() == initial);
    player.x += 0.5f;
    assert(bridge.commit(player));
    assert(world.revision().value() == initial.value() + 1);
    assert(world.entities().find(id)->transform.position.x == 1734);
    assert(world.entities().find(id)->transform.rotation.pitch == 1000);
    assert(world.entities().find(id)->transform.rotation.roll == -2000);
    const auto canonical = home::encode_snapshot(world.snapshot()).value();
    auto refused = bridge.submit(id, initial, player);
    assert(!refused && refused.error().code == home::ErrorCode::RevisionConflict);
    assert(!bridge.submit(home::EntityId{999}, world.revision(), player));
    for (float bad : {std::numeric_limits<float>::quiet_NaN(),
                      std::numeric_limits<float>::infinity(), 10001.0f}) {
        auto invalid = player;
        invalid.x = bad;
        assert(!bridge.commit(invalid));
        assert(home::encode_snapshot(world.snapshot()).value() == canonical);
    }
    auto invalidYaw = player;
    invalidYaw.yaw = std::numeric_limits<float>::infinity();
    assert(!bridge.commit(invalidYaw));
    assert(home::encode_snapshot(world.snapshot()).value() == canonical);

    auto external = world.entities().find(id)->transform;
    external.position.z += 1000;
    assert(world.update_transform(id, external));
    const auto externalBytes = home::encode_snapshot(world.snapshot()).value();
    refused = bridge.commit(player);
    assert(!refused && refused.error().code == home::ErrorCode::RevisionConflict);
    assert(home::encode_snapshot(world.snapshot()).value() == externalBytes);
    assert(bridge.load(player)); // Explicit reload, never a hidden stale retry.
    assert(std::abs(player.z - 4.125f) < 0.00001f);
    assert(bridge.commit(player));
    assert(home::encode_snapshot(world.snapshot()).value() == externalBytes);
    PlayerState untouched = player;
    HomeBridge missing{world, home::EntityId{999}};
    assert(!missing.load(player));
    assert(player.x == untouched.x && player.z == untouched.z);

    const auto root = std::filesystem::temp_directory_path() /
        ("hakui-home-spec-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto path = root / "player.snapshot";
    GameRuntime runtime;
    runtime.resetPlayerToSpawn();
    auto opened = HomeSession::open(path, runtime.player());
    assert(opened);
    auto session = std::move(opened).value();
    runtime.player() = movedPlayer();
    assert(session->commit(runtime.player()));
    assert(session->save());
    assert(!HomeSession::open(path, runtime.player())); // Second writer refused.
    const auto saved = bytes(path);
    const auto revision = session->world().revision();
    const auto entity = session->entity();
    session.reset();
    GameRuntime restarted;
    auto restored = HomeSession::open(path, restarted.player());
    assert(restored);
    session = std::move(restored).value();
    sameTransform(restarted.player(), HomeBridge::encode(movedPlayer()).value());
    assert(session->world().revision() == revision);
    assert(session->entity() == entity);
    assert(session->save());
    assert(bytes(path) == saved);
    // Save write failure must preserve the last committed disk image.
    auto pending = path;
    pending += ".pending";
    std::filesystem::create_directory(pending);
    assert(!session->save());
    assert(bytes(path) == saved);
    std::filesystem::remove(pending);
    session.reset();
    std::ofstream(path) << "corrupt save";
    const auto corrupt = bytes(path);
    assert(!HomeSession::open(path, restarted.player()));
    assert(bytes(path) == corrupt);
    // A valid HOME snapshot without the explicit binding must also fail closed.
    assert(home::save_snapshot_file(world.snapshot(), path.string()));
    assert(!HomeSession::open(path, restarted.player()));
    std::filesystem::remove_all(root);
    std::cout << "HOME bridge acceptance and rejection contracts passed\n";
}
