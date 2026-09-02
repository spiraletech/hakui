#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "spiral/hakui/HakuiAdapter.hpp"

namespace hakui {

struct SpiralPresenceView {
    bool linked = false;
    bool readOnly = true;
    bool cortexBound = false;
    bool playerInRange = false;

    std::uint32_t snapshotVersion = HakuiSnapshot::schemaVersion;
    float nodeX = 0.0f;
    float nodeY = 0.0f;
    float nodeZ = 0.0f;
    float playerDistance = 0.0f;
    float interactionRadius = 0.0f;
    std::size_t nearbyObjectCount = 0;

    std::string headline;
    std::string linkLine;
    std::string worldLine;
    std::string playerLine;
    std::string nearbyLine;
    std::string cortexLine;
};

// L8 visible, read-only embodiment of the HAKUI observation link.
//
// SpiralPresence owns no game state and accepts only the L7 read-only adapter.
// It formats semantic truth for presentation and exposes a stable authored node
// identity in the Black Room. L9 may bind a conversational cortex behind this
// presence, but L8 deliberately keeps cortexBound=false and has no action path.
class SpiralPresence final {
public:
    static constexpr std::uint32_t nodeAffordanceId = 1401;
    static constexpr float defaultNearbyRadius = 6.0f;
    static constexpr float nodeInteractionRadius = 2.35f;

    explicit SpiralPresence(const HakuiAdapter& adapter) noexcept
        : adapter_(adapter)
    {
    }

    SpiralPresence(HakuiAdapter&&) = delete;
    SpiralPresence(const HakuiAdapter&&) = delete;

    [[nodiscard]] const HakuiAdapter& adapter() const noexcept { return adapter_; }
    [[nodiscard]] SpiralPresenceView view(
        float nearbyRadius = defaultNearbyRadius
    ) const;

private:
    const HakuiAdapter& adapter_;
};

} // namespace hakui
