#include "spiral/hakui/SpiralCortexClient.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace hakui {
namespace {

constexpr std::array<char, 8> kRequestMagic{{'S','P','H','K','U','I','0','1'}};
constexpr std::array<char, 8> kResponseMagic{{'S','P','H','R','E','P','0','1'}};
constexpr std::uint32_t kPing = 1;
constexpr std::uint32_t kAsk = 2;
constexpr std::uint32_t kResponseOk = 0;
constexpr std::uint32_t kFlagLocalModelLoaded = 1U << 0U;
constexpr std::size_t kMaxContextBytes = 64U * 1024U;
constexpr std::size_t kMaxPromptBytes = 8U * 1024U;
constexpr std::size_t kMaxReplyBytes = 1024U * 1024U;

#ifdef _WIN32
using SocketHandle = SOCKET;
constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;
#else
using SocketHandle = int;
constexpr SocketHandle kInvalidSocket = -1;
#endif

class SocketRuntime final {
public:
    SocketRuntime() {
#ifdef _WIN32
        WSADATA data{};
        ok_ = WSAStartup(MAKEWORD(2, 2), &data) == 0;
#else
        ok_ = true;
#endif
    }

    ~SocketRuntime() {
#ifdef _WIN32
        if (ok_) WSACleanup();
#endif
    }

    SocketRuntime(const SocketRuntime&) = delete;
    SocketRuntime& operator=(const SocketRuntime&) = delete;

    [[nodiscard]] bool ok() const noexcept { return ok_; }

private:
    bool ok_ = false;
};

void close_socket(SocketHandle socket) noexcept
{
    if (socket == kInvalidSocket) return;
#ifdef _WIN32
    closesocket(socket);
#else
    close(socket);
#endif
}

void set_timeout(SocketHandle socket, int milliseconds) noexcept
{
    const int safeMilliseconds = std::max(1, milliseconds);
#ifdef _WIN32
    const DWORD timeout = static_cast<DWORD>(safeMilliseconds);
    (void)setsockopt(
        socket,
        SOL_SOCKET,
        SO_RCVTIMEO,
        reinterpret_cast<const char*>(&timeout),
        sizeof(timeout)
    );
    (void)setsockopt(
        socket,
        SOL_SOCKET,
        SO_SNDTIMEO,
        reinterpret_cast<const char*>(&timeout),
        sizeof(timeout)
    );
#else
    timeval timeout{};
    timeout.tv_sec = safeMilliseconds / 1000;
    timeout.tv_usec = (safeMilliseconds % 1000) * 1000;
    (void)setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    (void)setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif
}

bool send_all(SocketHandle socket, const void* data, std::size_t size)
{
    const auto* bytes = static_cast<const char*>(data);
    std::size_t sent = 0;
    while (sent < size) {
        const std::size_t remaining = size - sent;
        const int chunk = static_cast<int>(std::min<std::size_t>(
            remaining,
            static_cast<std::size_t>(std::numeric_limits<int>::max())
        ));
#ifdef _WIN32
        const int written = send(socket, bytes + sent, chunk, 0);
#else
        const int written = static_cast<int>(send(
            socket,
            bytes + sent,
            static_cast<std::size_t>(chunk),
            0
        ));
#endif
        if (written <= 0) return false;
        sent += static_cast<std::size_t>(written);
    }
    return true;
}

bool receive_all(SocketHandle socket, void* data, std::size_t size)
{
    auto* bytes = static_cast<char*>(data);
    std::size_t received = 0;
    while (received < size) {
        const std::size_t remaining = size - received;
        const int chunk = static_cast<int>(std::min<std::size_t>(
            remaining,
            static_cast<std::size_t>(std::numeric_limits<int>::max())
        ));
#ifdef _WIN32
        const int read = recv(socket, bytes + received, chunk, 0);
#else
        const int read = static_cast<int>(recv(
            socket,
            bytes + received,
            static_cast<std::size_t>(chunk),
            0
        ));
#endif
        if (read <= 0) return false;
        received += static_cast<std::size_t>(read);
    }
    return true;
}

bool send_u32(SocketHandle socket, std::uint32_t value)
{
    const std::uint32_t wire = htonl(value);
    return send_all(socket, &wire, sizeof(wire));
}

bool receive_u32(SocketHandle socket, std::uint32_t& value)
{
    std::uint32_t wire = 0;
    if (!receive_all(socket, &wire, sizeof(wire))) return false;
    value = ntohl(wire);
    return true;
}

std::string sanitize_line(std::string_view value)
{
    std::string out;
    out.reserve(value.size());
    for (char c : value) {
        if (c == '\n' || c == '\r' || c == '\t') {
            out.push_back(' ');
        } else {
            out.push_back(c);
        }
    }
    return out;
}

} // namespace

SpiralCortexReply SpiralCortexClient::probe() const
{
    return transact(kPing, {}, {}, endpoint_.probeTimeoutMilliseconds);
}

SpiralCortexReply SpiralCortexClient::ask(
    const HakuiSnapshot& snapshot,
    std::string_view prompt
) const
{
    if (prompt.empty()) {
        SpiralCortexReply reply;
        reply.error = "empty cortex prompt";
        return reply;
    }
    if (prompt.size() > kMaxPromptBytes) {
        SpiralCortexReply reply;
        reply.error = "cortex prompt exceeds L9 wire limit";
        return reply;
    }

    const std::string context = buildReadOnlyContext(snapshot);
    if (context.size() > kMaxContextBytes) {
        SpiralCortexReply reply;
        reply.error = "HAKUI observation context exceeds L9 wire limit";
        return reply;
    }
    return transact(
        kAsk,
        context,
        prompt,
        endpoint_.requestTimeoutMilliseconds
    );
}

std::string SpiralCortexClient::buildReadOnlyContext(
    const HakuiSnapshot& snapshot
)
{
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(3);
    stream << "SPIRAL_HAKUI_CONTEXT/1\n";
    stream << "authority=read_only\n";
    stream << "snapshot.version=" << snapshot.version << '\n';
    stream << "world.id=" << sanitize_line(snapshot.world.worldId) << '\n';
    stream << "world.step=" << snapshot.world.simulationStep << '\n';
    stream << "world.elapsed_seconds=" << snapshot.world.elapsedSeconds << '\n';
    stream << "world.affordance_count=" << snapshot.world.affordances.size() << '\n';
    stream << "world.seat_count=" << snapshot.world.seats.size() << '\n';
    stream << "world.occupied_seat_count=" << snapshot.world.occupiedSeatCount << '\n';
    stream << "player.name=" << sanitize_line(snapshot.player.displayName) << '\n';
    stream << "player.position="
           << snapshot.player.x << ','
           << snapshot.player.y << ','
           << snapshot.player.z << '\n';
    stream << "player.yaw=" << snapshot.player.yaw << '\n';
    stream << "player.velocity="
           << snapshot.player.velocityX << ','
           << snapshot.player.velocityY << ','
           << snapshot.player.velocityZ << '\n';
    stream << "player.health=" << snapshot.player.health << '\n';
    stream << "player.hunger=" << snapshot.player.hunger << '\n';
    stream << "player.stamina=" << snapshot.player.stamina << '\n';
    stream << "player.money=" << snapshot.player.money << '\n';
    stream << "player.grounded=" << (snapshot.player.grounded ? "true" : "false") << '\n';
    stream << "player.sprinting=" << (snapshot.player.sprinting ? "true" : "false") << '\n';
    stream << "player.active_affordance_id=" << snapshot.player.activeAffordanceId << '\n';
    stream << "player.active_seat_id=" << snapshot.player.activeSeatAnchorId << '\n';
    stream << "interaction.live_target_count="
           << snapshot.interactions.liveTargetIds.size() << '\n';

    stream << "npc.count=" << snapshot.npcs.size() << '\n';
    for (std::size_t index = 0; index < snapshot.npcs.size(); ++index) {
        const HakuiNpcSnapshot& npc = snapshot.npcs[index];
        stream << "npc." << index << ".id=" << npc.id << '\n';
        stream << "npc." << index << ".name="
               << sanitize_line(npc.displayName) << '\n';
        stream << "npc." << index << ".position="
               << npc.x << ',' << npc.y << ',' << npc.z << '\n';
        stream << "npc." << index << ".yaw=" << npc.yaw << '\n';
        stream << "npc." << index << ".activity="
               << static_cast<unsigned>(npc.activity) << '\n';
        stream << "npc." << index << ".mood="
               << static_cast<unsigned>(npc.mood) << '\n';
        stream << "npc." << index << ".routine="
               << static_cast<unsigned>(npc.routine) << '\n';
        stream << "npc." << index << ".active_affordance_id="
               << npc.activeAffordanceId << '\n';
        stream << "npc." << index << ".active_seat_id="
               << npc.activeSeatAnchorId << '\n';
        stream << "npc." << index << ".needs="
               << npc.needs.hunger << ','
               << npc.needs.energy << ','
               << npc.needs.social << ','
               << npc.needs.comfort << ','
               << npc.needs.fun << '\n';
    }

    for (std::size_t index = 0; index < snapshot.world.affordances.size(); ++index) {
        const HakuiAffordanceSnapshot& affordance = snapshot.world.affordances[index];
        stream << "affordance." << index << ".id=" << affordance.id << '\n';
        stream << "affordance." << index << ".label="
               << sanitize_line(affordance.label) << '\n';
        stream << "affordance." << index << ".mask=" << affordance.affordances << '\n';
        stream << "affordance." << index << ".anchor="
               << affordance.primaryAnchor.x << ','
               << affordance.primaryAnchor.y << ','
               << affordance.primaryAnchor.z << '\n';
    }

    stream << "policy=observe_and_reply_only\n";
    stream << "policy.no_world_mutation=true\n";
    stream << "policy.no_player_control=true\n";
    stream << "policy.no_interaction_execution=true\n";
    return stream.str();
}

SpiralCortexReply SpiralCortexClient::transact(
    std::uint32_t requestKind,
    std::string_view context,
    std::string_view prompt,
    int timeoutMilliseconds
) const
{
    SpiralCortexReply reply;
    if (context.size() > std::numeric_limits<std::uint32_t>::max() ||
        prompt.size() > std::numeric_limits<std::uint32_t>::max()) {
        reply.error = "cortex request size overflow";
        return reply;
    }

    SocketRuntime sockets;
    if (!sockets.ok()) {
        reply.error = "socket runtime unavailable";
        return reply;
    }

    SocketHandle socketHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socketHandle == kInvalidSocket) {
        reply.error = "cortex socket creation failed";
        return reply;
    }
    set_timeout(socketHandle, timeoutMilliseconds);

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(endpoint_.port);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(
            socketHandle,
            reinterpret_cast<const sockaddr*>(&address),
            sizeof(address)) != 0) {
        close_socket(socketHandle);
        reply.error = "SpiralHakuiCortex is not listening on loopback";
        return reply;
    }
    reply.connected = true;

    const bool sent =
        send_all(socketHandle, kRequestMagic.data(), kRequestMagic.size()) &&
        send_u32(socketHandle, requestKind) &&
        send_u32(socketHandle, static_cast<std::uint32_t>(context.size())) &&
        send_u32(socketHandle, static_cast<std::uint32_t>(prompt.size())) &&
        (context.empty() || send_all(socketHandle, context.data(), context.size())) &&
        (prompt.empty() || send_all(socketHandle, prompt.data(), prompt.size()));
    if (!sent) {
        close_socket(socketHandle);
        reply.error = "cortex request transmission failed";
        return reply;
    }

    std::array<char, 8> magic{};
    std::uint32_t status = 0;
    std::uint32_t flags = 0;
    std::uint32_t modelBytes = 0;
    std::uint32_t textBytes = 0;
    if (!receive_all(socketHandle, magic.data(), magic.size()) ||
        magic != kResponseMagic ||
        !receive_u32(socketHandle, status) ||
        !receive_u32(socketHandle, flags) ||
        !receive_u32(socketHandle, modelBytes) ||
        !receive_u32(socketHandle, textBytes)) {
        close_socket(socketHandle);
        reply.error = "invalid or timed-out cortex response";
        return reply;
    }

    if (modelBytes > kMaxContextBytes || textBytes > kMaxReplyBytes) {
        close_socket(socketHandle);
        reply.error = "cortex response exceeds L9 wire limit";
        return reply;
    }

    reply.model.assign(modelBytes, '\0');
    reply.text.assign(textBytes, '\0');
    if ((modelBytes > 0 &&
         !receive_all(socketHandle, reply.model.data(), reply.model.size())) ||
        (textBytes > 0 &&
         !receive_all(socketHandle, reply.text.data(), reply.text.size()))) {
        close_socket(socketHandle);
        reply.error = "incomplete cortex response payload";
        return reply;
    }
    close_socket(socketHandle);

    reply.localModelLoaded = (flags & kFlagLocalModelLoaded) != 0U;
    reply.ok = status == kResponseOk;
    if (!reply.ok) {
        reply.error = reply.text.empty() ? "Spiral cortex runtime error" : reply.text;
        reply.text.clear();
    }
    return reply;
}

} // namespace hakui
