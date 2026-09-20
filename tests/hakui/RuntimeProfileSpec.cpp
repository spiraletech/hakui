#include "core/NativeRuntimeProfile.hpp"

#include <cassert>

int main() {
    using hakui::NativeRuntimeProfile;

    assert(hakui::parse_native_runtime_profile("") == NativeRuntimeProfile::Core);
    assert(hakui::parse_native_runtime_profile("core") == NativeRuntimeProfile::Core);
    assert(hakui::parse_native_runtime_profile("unknown") == NativeRuntimeProfile::Core);
    assert(hakui::parse_native_runtime_profile("full") == NativeRuntimeProfile::Full);
    assert(hakui::parse_native_runtime_profile("legacy") == NativeRuntimeProfile::Full);

    const auto core = hakui::native_runtime_features(NativeRuntimeProfile::Core);
    assert(!core.cortex);
    assert(!core.tabletop);
    assert(!core.authored_story);
    assert(!core.ambient_npcs);

    const auto full = hakui::native_runtime_features(NativeRuntimeProfile::Full);
    assert(full.cortex);
    assert(full.tabletop);
    assert(full.authored_story);
    assert(full.ambient_npcs);

    return 0;
}
