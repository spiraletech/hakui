#pragma once

#include "home/hakui_adapter.hpp"
#include "player/PlayerState.hpp"

namespace hakui {

// One explicitly bound HOME entity; never assumes HAKUI's local player id is
// a HOME id. HAKUI units are metres, yaw is radians. HOME uses mm/millidegrees.
class HomeBridge final {
public:
    HomeBridge(home::VersionedWorld& world, home::EntityId entity)
        : world_(world), entity_(entity) {}

    home::Result<void> load(PlayerState& player);
    home::Result<void> commit(const PlayerState& player);
    home::Result<void> submit(home::EntityId entity,
                              home::WorldRevision expected,
                              const PlayerState& candidate);
    home::WorldRevision observedRevision() const { return observed_; }
    home::EntityId entity() const { return entity_; }
    static home::Result<home::Transform> encode(const PlayerState& player,
                                               home::Transform base = {});
private:
    home::VersionedWorld& world_;
    home::EntityId entity_;
    home::WorldRevision observed_{};
    bool loaded_ = false;
};
} // namespace hakui
