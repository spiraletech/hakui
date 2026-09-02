#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "core/HakuiSnapshot.hpp"

namespace hakui {

enum class HakuiAdapterStatus : std::uint8_t {
    Ok,
    UnsupportedCommand,
    InvalidArgument
};

struct HakuiTimeInspection {
    float elapsedSeconds = 0.0f;
    std::uint64_t simulationStep = 0;
};

struct HakuiNearbyObservation {
    std::uint32_t affordanceId = 0;
    std::string label;
    WorldAffordanceMask affordances = 0;
    float distance = 0.0f;
    HakuiAnchorSnapshot primaryAnchor;
    std::size_t seatCount = 0;
    std::size_t occupiedSeatCount = 0;
};

struct HakuiNearbyInspection {
    float radius = 0.0f;
    std::vector<HakuiNearbyObservation> objects;
};

using HakuiAdapterPayload = std::variant<
    std::monostate,
    HakuiWorldSnapshot,
    HakuiPlayerSnapshot,
    HakuiNearbyInspection,
    HakuiInteractionSnapshot,
    HakuiTimeInspection
>;

struct HakuiAdapterResult {
    HakuiAdapterStatus status = HakuiAdapterStatus::Ok;
    std::string command;
    std::uint32_t snapshotVersion = HakuiSnapshot::schemaVersion;
    std::string message;
    HakuiAdapterPayload payload;

    explicit operator bool() const noexcept
    {
        return status == HakuiAdapterStatus::Ok;
    }
};

// L7 read-only semantic bridge over the L6 snapshot contract.
//
// The adapter intentionally stores only a const runtime reference. It cannot
// mutate simulation state, reserve seats, execute interactions, move the
// player, or write Spiral state. Each inspection captures one frozen L6
// snapshot and derives a typed observation from that packet.
class HakuiAdapter final {
public:
    static constexpr std::string_view inspectWorldCommand = "hakui.inspect_world";
    static constexpr std::string_view inspectPlayerCommand = "hakui.inspect_player";
    static constexpr std::string_view inspectNearbyCommand = "hakui.inspect_nearby";
    static constexpr std::string_view inspectTimeCommand = "hakui.inspect_time";
    static constexpr std::string_view inspectInteractionsCommand = "hakui.inspect_interactions";

    static constexpr std::array<std::string_view, 5> readOnlyCommands{{
        inspectWorldCommand,
        inspectPlayerCommand,
        inspectNearbyCommand,
        inspectTimeCommand,
        inspectInteractionsCommand
    }};

    explicit HakuiAdapter(const GameRuntime& runtime) noexcept
        : runtime_(runtime)
    {
    }

    // The adapter stores a reference; binding it to a temporary runtime would
    // create a dangling observation source after the full expression.
    HakuiAdapter(GameRuntime&&) = delete;
    HakuiAdapter(const GameRuntime&&) = delete;

    [[nodiscard]] const GameRuntime& runtime() const noexcept { return runtime_; }
    [[nodiscard]] HakuiSnapshot snapshot() const;

    [[nodiscard]] HakuiWorldSnapshot inspectWorld() const;
    [[nodiscard]] HakuiPlayerSnapshot inspectPlayer() const;
    [[nodiscard]] HakuiNearbyInspection inspectNearby(float radius = 6.0f) const;
    [[nodiscard]] HakuiTimeInspection inspectTime() const;
    [[nodiscard]] HakuiInteractionSnapshot inspectInteractions() const;

    [[nodiscard]] bool supports(std::string_view command) const noexcept;
    [[nodiscard]] HakuiAdapterResult execute(
        std::string_view command,
        float nearbyRadius = 6.0f
    ) const;

private:
    const GameRuntime& runtime_;
};

} // namespace hakui
