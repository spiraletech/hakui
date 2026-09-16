#include "home/HomeBridge.hpp"

#include <cmath>
#include <numbers>

namespace hakui {
namespace {
constexpr double maxMetres = 10000.0;
constexpr double degreesPerRadian = 180.0 / std::numbers::pi;
home::Result<void> fail(home::ErrorCode code, const char* message) {
    return home::Result<void>::failure(code, message);
}
}

home::Result<home::Transform> HomeBridge::encode(const PlayerState& p, home::Transform t) {
    for (float v : {p.x, p.y, p.z, p.yaw}) {
        if (!std::isfinite(v))
            return home::Result<home::Transform>::failure(
                home::ErrorCode::ValidationFailed, "non-finite HAKUI transform");
    }
    if (std::abs(p.x) > maxMetres || std::abs(p.y) > maxMetres || std::abs(p.z) > maxMetres)
        return home::Result<home::Transform>::failure(
            home::ErrorCode::ValidationFailed, "transform exceeds the local bridge domain");
    t.position = {std::llround(double(p.x) * 1000.0),
                  std::llround(double(p.y) * 1000.0),
                  std::llround(double(p.z) * 1000.0)};
    // Normalize before narrowing. Preserve HOME pitch/roll, which this player
    // controller does not simulate.
    t.rotation.yaw = static_cast<std::int32_t>(std::llround(
        std::remainder(double(p.yaw), 2.0 * std::numbers::pi) * degreesPerRadian * 1000.0));
    return home::Result<home::Transform>::success(t);
}

home::Result<void> HomeBridge::load(PlayerState& player) {
    home::HakuiAdapter adapter{world_};
    const auto frame = adapter.project();
    for (const auto& body : frame.bodies) {
        if (body.entity != entity_) continue;
        if (body.kind != home::EntityKind::Avatar || !body.persistent)
            return fail(home::ErrorCode::ValidationFailed, "bound entity must be a persistent avatar");
        const auto& t = body.transform;
        for (auto v : {t.position.x, t.position.y, t.position.z})
            if (v < -10000000 || v > 10000000)
                return fail(home::ErrorCode::ValidationFailed, "saved transform exceeds the local bridge domain");
        // No partially applied projection: validate everything first.
        player.x = static_cast<float>(double(t.position.x) / 1000.0);
        player.y = static_cast<float>(double(t.position.y) / 1000.0);
        player.z = static_cast<float>(double(t.position.z) / 1000.0);
        player.yaw = static_cast<float>(double(t.rotation.yaw) / (1000.0 * degreesPerRadian));
        player.velocityX = player.velocityY = player.velocityZ = 0.0f;
        observed_ = frame.revision;
        loaded_ = true;
        return home::Result<void>::success();
    }
    return fail(home::ErrorCode::NotFound, "bound HOME entity is missing");
}

home::Result<void> HomeBridge::commit(const PlayerState& player) {
    return submit(entity_, observed_, player);
}

home::Result<void> HomeBridge::submit(home::EntityId entity,
                                     home::WorldRevision expected,
                                     const PlayerState& candidate) {
    if (!loaded_)
        return fail(home::ErrorCode::ValidationFailed, "load HOME projection before submitting movement");
    if (entity != entity_)
        return fail(home::ErrorCode::ValidationFailed, "movement is outside the bound entity scope");
    if (expected != observed_ || expected != world_.revision())
        return fail(home::ErrorCode::RevisionConflict, "stale HOME movement revision; reload required");
    const auto* record = world_.entities().find(entity_);
    if (!record)
        return fail(home::ErrorCode::NotFound, "bound HOME entity is missing");
    const auto encoded = encode(candidate, record->transform);
    if (!encoded) return home::Result<void>::failure(encoded.error().code, encoded.error().message);
    // Sub-millimetre motion is a no-op, not an empty transaction or new revision.
    if (encoded.value() == record->transform) return home::Result<void>::success();
    const auto next = expected.next();
    if (!next) return fail(home::ErrorCode::Overflow, "HOME revision exhausted");
    home::HakuiConsequenceBatch batch{};
    batch.id = home::WorldTransactionId{next->value()};
    batch.expected_revision = expected;
    batch.authority = "hakui.local-player";
    batch.consequences.emplace_back(home::HakuiTransformConsequence{entity_, encoded.value()});
    home::HakuiAdapter adapter{world_};
    const auto receipt = adapter.apply(batch);
    if (!receipt) return home::Result<void>::failure(receipt.error().code, receipt.error().message);
    observed_ = receipt.value().to_revision;
    return home::Result<void>::success();
}
} // namespace hakui
