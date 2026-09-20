#pragma once

#include <string_view>

namespace hakui {

enum class NativeRuntimeProfile {
    Core,
    Full,
};

struct NativeRuntimeFeatures {
    bool cortex{false};
    bool tabletop{false};
    bool authored_story{false};
    bool ambient_npcs{false};
};

[[nodiscard]] constexpr NativeRuntimeProfile parse_native_runtime_profile(
    std::string_view value
) noexcept {
    return (value == "full" || value == "legacy" || value == "experimental")
        ? NativeRuntimeProfile::Full
        : NativeRuntimeProfile::Core;
}

[[nodiscard]] constexpr NativeRuntimeFeatures native_runtime_features(
    NativeRuntimeProfile profile
) noexcept {
    if (profile == NativeRuntimeProfile::Full) {
        return {true, true, true, true};
    }
    return {};
}

[[nodiscard]] constexpr bool native_runtime_full(
    NativeRuntimeProfile profile
) noexcept {
    return profile == NativeRuntimeProfile::Full;
}

[[nodiscard]] constexpr std::string_view to_string(
    NativeRuntimeProfile profile
) noexcept {
    switch (profile) {
        case NativeRuntimeProfile::Core: return "core";
        case NativeRuntimeProfile::Full: return "full";
    }
    return "core";
}

} // namespace hakui
