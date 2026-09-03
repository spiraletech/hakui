#pragma once

#include <cstdint>
#include <string_view>

namespace hakui {

class GameRuntime;

enum class HakuiActionSource : std::uint8_t {
    Cortex,
    LocalSystem
};

enum class HakuiActionVerb : std::uint8_t {
    NpcObservePlayer,
    NpcResumeRoutine
};

enum class HakuiActionCapability : std::uint32_t {
    None = 0,
    NpcAttention = 1U << 0U
};

using HakuiActionCapabilityMask = std::uint32_t;

[[nodiscard]] constexpr HakuiActionCapabilityMask actionCapability(
    HakuiActionCapability capability
) noexcept
{
    return static_cast<HakuiActionCapabilityMask>(capability);
}

struct HakuiActionRequest {
    std::uint64_t requestId = 0;
    HakuiActionSource source = HakuiActionSource::Cortex;
    HakuiActionVerb verb = HakuiActionVerb::NpcObservePlayer;
    std::uint32_t targetNpcId = 0;
    std::uint32_t snapshotVersion = 0;
    std::uint64_t observedWorldStep = 0;
};

// A grant is supplied by trusted HAKUI policy code, never by cortex text. It is
// deliberately narrow: one source, one target, one capability and one bounded
// world-step window.
struct HakuiActionGrant {
    HakuiActionSource source = HakuiActionSource::LocalSystem;
    std::uint32_t targetNpcId = 0;
    HakuiActionCapabilityMask capabilities = 0;
    std::uint64_t validFromWorldStep = 0;
    std::uint64_t validThroughWorldStep = 0;
};

enum class HakuiActionStatus : std::uint8_t {
    Executed,
    InvalidRequest,
    PermissionDenied,
    StaleObservation,
    UnknownTarget,
    RejectedByAuthority
};

struct HakuiActionResult {
    std::uint64_t requestId = 0;
    HakuiActionStatus status = HakuiActionStatus::InvalidRequest;
    std::string_view detail;

    [[nodiscard]] bool executed() const noexcept
    {
        return status == HakuiActionStatus::Executed;
    }
};

struct HakuiActionAudit {
    std::uint64_t evaluated = 0;
    std::uint64_t executed = 0;
    std::uint64_t denied = 0;
    std::uint64_t lastRequestId = 0;
    HakuiActionStatus lastStatus = HakuiActionStatus::InvalidRequest;
};

// L11 is the only cognition-to-reality mutation boundary. It accepts typed
// requests, validates explicit host grants and freshness, then delegates the
// actual state change to its owning gameplay authority.
class HakuiActionGate final {
public:
    [[nodiscard]] HakuiActionResult execute(
        GameRuntime& runtime,
        const HakuiActionRequest& request,
        const HakuiActionGrant& grant
    ) noexcept;

    [[nodiscard]] const HakuiActionAudit& audit() const noexcept
    {
        return audit_;
    }

    void resetAudit() noexcept { audit_ = {}; }

private:
    [[nodiscard]] static HakuiActionCapability requiredCapability(
        HakuiActionVerb verb
    ) noexcept;
    HakuiActionResult finish(
        const HakuiActionRequest& request,
        HakuiActionStatus status,
        std::string_view detail
    ) noexcept;

    HakuiActionAudit audit_{};
};

} // namespace hakui
