#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "core/HakuiSnapshot.hpp"

namespace hakui {

struct SpiralCortexEndpoint {
    // L9 deliberately binds only to the local machine. Remote/network AI
    // authority is outside this layer's contract.
    std::uint16_t port = 47691;
    int probeTimeoutMilliseconds = 180;
    int requestTimeoutMilliseconds = 30000;
};

struct SpiralCortexStatus {
    bool bound = false;
    bool localModelLoaded = false;
    bool busy = false;
    std::string runtimeName = "Spiral Ether AI";
    std::string model;
    std::string detail;
};

struct SpiralCortexReply {
    bool connected = false;
    bool ok = false;
    bool localModelLoaded = false;
    std::string model;
    std::string text;
    std::string error;
};

// L9 client for the real Spiral Ether AI HAKUI host process.
//
// This object is intentionally stateless and copyable. Every transaction opens
// a short-lived loopback socket to SpiralHakuiCortex, sends a frozen L6
// HakuiSnapshot context plus optional human prompt, receives text, and closes.
// It exposes no HAKUI write commands and never owns GameRuntime authority.
class SpiralCortexClient final {
public:
    static constexpr std::uint16_t defaultPort = 47691;
    static constexpr std::string_view protocolName = "SPIRAL_HAKUI/1";

    explicit SpiralCortexClient(SpiralCortexEndpoint endpoint = {}) noexcept
        : endpoint_(endpoint)
    {
    }

    [[nodiscard]] SpiralCortexEndpoint endpoint() const noexcept
    {
        return endpoint_;
    }

    [[nodiscard]] SpiralCortexReply probe() const;
    [[nodiscard]] SpiralCortexReply ask(
        const HakuiSnapshot& snapshot,
        std::string_view prompt
    ) const;

    [[nodiscard]] static std::string buildReadOnlyContext(
        const HakuiSnapshot& snapshot
    );

private:
    [[nodiscard]] SpiralCortexReply transact(
        std::uint32_t requestKind,
        std::string_view context,
        std::string_view prompt,
        int timeoutMilliseconds
    ) const;

    SpiralCortexEndpoint endpoint_{};
};

} // namespace hakui
