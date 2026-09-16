#pragma once

#include "home/HomeBridge.hpp"
#include <filesystem>
#include <memory>

namespace hakui {
class HomeSaveLock;
// A single-process local HOME session. The save contains canonical HOME data,
// not a second HAKUI persistence schema. No GUI or physics types enter HOME.
class HomeSession final {
public:
    ~HomeSession();
    static home::Result<std::unique_ptr<HomeSession>> open(
        const std::filesystem::path& path, PlayerState& player);
    home::Result<void> commit(const PlayerState& player) { return bridge_.commit(player); }
    home::Result<void> reload(PlayerState& player) { return bridge_.load(player); }
    home::Result<void> save();
    const home::VersionedWorld& world() const { return world_; }
    home::EntityId entity() const { return bridge_.entity(); }
private:
    HomeSession(std::filesystem::path path, home::VersionedWorld world, home::EntityId entity,
                std::unique_ptr<HomeSaveLock> lock);
    std::unique_ptr<HomeSaveLock> lock_;
    std::filesystem::path path_;
    home::VersionedWorld world_;
    HomeBridge bridge_;
};
} // namespace hakui
