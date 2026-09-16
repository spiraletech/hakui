#include "home/HomeSession.hpp"
#include "home/snapshot.hpp"

#include <fstream>
#include <iterator>
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace hakui {
// OS-held lock survives neither a crash nor normal shutdown. The lock file may
// remain, but only the live OS handle confers ownership; no stale-file guessing.
class HomeSaveLock final {
public:
    explicit HomeSaveLock(const std::filesystem::path& save) {
        auto path = save;
        path += ".lock";
#ifdef _WIN32
        handle_ = CreateFileW(path.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr,
                              OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
#else
        handle_ = ::open(path.c_str(), O_CREAT | O_RDWR | O_CLOEXEC, 0600);
        if (handle_ >= 0 && flock(handle_, LOCK_EX | LOCK_NB) != 0) {
            close(handle_);
            handle_ = -1;
        }
#endif
    }
    bool valid() const {
#ifdef _WIN32
        return handle_ != INVALID_HANDLE_VALUE;
#else
        return handle_ >= 0;
#endif
    }
    ~HomeSaveLock() {
        if (!valid()) return;
#ifdef _WIN32
        CloseHandle(handle_);
#else
        close(handle_);
#endif
    }
private:
#ifdef _WIN32
    HANDLE handle_ = INVALID_HANDLE_VALUE;
#else
    int handle_ = -1;
#endif
};

HomeSession::HomeSession(std::filesystem::path path, home::VersionedWorld world,
                         home::EntityId entity, std::unique_ptr<HomeSaveLock> lock)
    : lock_(std::move(lock)), path_(std::move(path)), world_(std::move(world)), bridge_(world_, entity) {}
HomeSession::~HomeSession() = default;

namespace {
constexpr auto playerArchetype = "hakui.local-player.v1";
using OpenResult = home::Result<std::unique_ptr<HomeSession>>;
}

OpenResult HomeSession::open(const std::filesystem::path& path, PlayerState& player) {
    if (path.empty()) return OpenResult::failure(home::ErrorCode::InvalidArgument, "empty HOME save path");
    std::error_code ec;
    if (!path.parent_path().empty()) std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) return OpenResult::failure(home::ErrorCode::SerializationError, ec.message());
    auto lock = std::make_unique<HomeSaveLock>(path);
    if (!lock->valid()) return OpenResult::failure(home::ErrorCode::SerializationError,
        "HOME save is already in use or its lock is unavailable");
    const bool exists = std::filesystem::exists(path, ec);
    if (ec) return OpenResult::failure(home::ErrorCode::SerializationError, ec.message());
    home::VersionedWorld world{home::WorldId{1}};
    home::EntityId entity{};
    if (exists) {
        const auto size = std::filesystem::file_size(path, ec);
        if (ec || size > 16 * 1024 * 1024)
            return OpenResult::failure(home::ErrorCode::SerializationError, "HOME save is unreadable or exceeds 16 MiB");
        std::ifstream input(path, std::ios::binary);
        if (!input) return OpenResult::failure(home::ErrorCode::SerializationError, "cannot open HOME save");
        const std::string bytes{std::istreambuf_iterator<char>{input}, {}};
        if (input.bad()) return OpenResult::failure(home::ErrorCode::SerializationError, "cannot read HOME save");
        const auto snapshot = home::decode_snapshot(bytes);
        if (!snapshot) return OpenResult::failure(snapshot.error().code, snapshot.error().message);
        auto restored = home::VersionedWorld::from_snapshot(snapshot.value());
        if (!restored) return OpenResult::failure(restored.error().code, restored.error().message);
        world = std::move(restored).value();
        for (const auto& record : world.entities().snapshot()) {
            if (record.archetype != playerArchetype) continue;
            if (entity.valid()) return OpenResult::failure(home::ErrorCode::ValidationFailed, "ambiguous HOME player binding");
            entity = record.id;
        }
        if (!entity.valid()) return OpenResult::failure(home::ErrorCode::NotFound, "HOME save has no bound HAKUI player");
    } else {
        const auto transform = HomeBridge::encode(player);
        if (!transform) return OpenResult::failure(transform.error().code, transform.error().message);
        home::EntityCreateInfo info{};
        info.kind = home::EntityKind::Avatar;
        info.archetype = playerArchetype;
        info.display_name = player.displayName;
        info.transform = transform.value();
        info.persistent = true;
        const auto created = world.create_entity(info);
        if (!created) return OpenResult::failure(created.error().code, created.error().message);
        entity = created.value();
    }
    auto session = std::unique_ptr<HomeSession>(new HomeSession(path, std::move(world), entity, std::move(lock)));
    const auto loaded = session->reload(player);
    if (!loaded) return OpenResult::failure(loaded.error().code, loaded.error().message);
    return OpenResult::success(std::move(session));
}

home::Result<void> HomeSession::save() {
    const auto encoded = home::encode_snapshot(world_.snapshot());
    if (!encoded) return home::Result<void>::failure(encoded.error().code, encoded.error().message);
    std::error_code ec;
    if (!path_.parent_path().empty()) std::filesystem::create_directories(path_.parent_path(), ec);
    if (ec) return home::Result<void>::failure(home::ErrorCode::SerializationError, ec.message());
    // Write beside the destination; a failed write cannot truncate the last save.
    auto temporary = path_;
    temporary += ".pending";
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        output.write(encoded.value().data(), static_cast<std::streamsize>(encoded.value().size()));
        output.close();
        if (!output) return home::Result<void>::failure(home::ErrorCode::SerializationError, "HOME save write failed");
    }
#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), path_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
        ec = std::error_code(static_cast<int>(GetLastError()), std::system_category());
#else
    std::filesystem::rename(temporary, path_, ec);
#endif
    if (ec) return home::Result<void>::failure(home::ErrorCode::SerializationError, "HOME save replacement failed: " + ec.message());
    return home::Result<void>::success();
}
} // namespace hakui
