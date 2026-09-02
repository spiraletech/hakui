#pragma once

#include <cmath>
#include <cstdint>
#include <string_view>

#include "world/BlackRoom.hpp"

namespace hakui {

enum class HakuiWorldScene : std::uint8_t {
    BlackRoom = 1
};

// Platform-neutral deterministic simulation clock.
//
// The float conversion and += operator intentionally preserve the small L3
// call-site surface (`world.elapsedSeconds`) while giving L4 one place to own
// simulation-step accounting and invalid-delta rejection. New code should use
// HakuiWorldState::clock()/advance().
class HakuiSimulationClock final {
public:
    constexpr float seconds() const noexcept { return seconds_; }
    constexpr std::uint64_t step() const noexcept { return step_; }

    constexpr operator float() const noexcept { return seconds_; }

    HakuiSimulationClock& operator+=(float deltaSeconds) noexcept
    {
        advance(deltaSeconds);
        return *this;
    }

    void advance(float deltaSeconds) noexcept
    {
        if (!std::isfinite(deltaSeconds) || deltaSeconds <= 0.0f) {
            return;
        }

        const float next = seconds_ + deltaSeconds;
        if (!std::isfinite(next)) {
            return;
        }

        seconds_ = next;
        ++step_;
    }

    constexpr void reset() noexcept
    {
        seconds_ = 0.0f;
        step_ = 0;
    }

private:
    float seconds_ = 0.0f;
    std::uint64_t step_ = 0;
};

// L4 canonical world-state contract.
//
// World identity, simulation time and the authored proof environment now live
// behind one deterministic authority. Presentation, SDL, audio, camera, chat,
// combat and Spiral orchestration remain outside this type.
class HakuiWorldState final {
public:
    static constexpr std::uint32_t schemaVersion = 1;
    static constexpr std::string_view canonicalWorldId = "hakui.black_room";
    static constexpr HakuiWorldScene canonicalScene = HakuiWorldScene::BlackRoom;

    HakuiWorldState() = default;
    HakuiWorldState(const HakuiWorldState&) = delete;
    HakuiWorldState& operator=(const HakuiWorldState&) = delete;
    HakuiWorldState(HakuiWorldState&&) = delete;
    HakuiWorldState& operator=(HakuiWorldState&&) = delete;

    HakuiSimulationClock& clock() noexcept { return elapsedSeconds; }
    const HakuiSimulationClock& clock() const noexcept { return elapsedSeconds; }

    BlackRoom& blackRoom() noexcept { return blackRoom_; }
    const BlackRoom& blackRoom() const noexcept { return blackRoom_; }

    void advance(float deltaSeconds) noexcept
    {
        elapsedSeconds.advance(deltaSeconds);
    }

    // Restore a fresh deterministic world session. Authored static geometry is
    // unchanged; mutable room state such as seat reservations is reconstructed.
    void reset() noexcept
    {
        elapsedSeconds.reset();
        blackRoom_ = BlackRoom{};
    }

    // Transitional L3-compatible clock surface. It is deliberately a
    // float-like first-party type rather than a naked mutable float.
    HakuiSimulationClock elapsedSeconds{};

private:
    BlackRoom blackRoom_{};
};

} // namespace hakui
