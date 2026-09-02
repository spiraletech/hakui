# PROJECT HAKUI :: VISIBLE SPIRAL PRESENCE
#
# Presentation-facing semantic presence over the read-only HakuiAdapter. This
# layer may format observation truth, but must remain independent from SDL,
# renderer implementation details, audio, model backends, and world mutation.

include(${CMAKE_CURRENT_LIST_DIR}/DependencyFirewall.cmake)

file(GLOB HAKUI_PRESENCE_FIREWALL_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../src/spiral/hakui/SpiralPresence.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/spiral/hakui/SpiralPresence.cpp"
)

hakui_enforce_first_party_firewall(
    "Hakui Spiral Presence"
    ${HAKUI_PRESENCE_FIREWALL_FILES}
)

add_library(hakui_presence STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/spiral/hakui/SpiralPresence.cpp
)

target_include_directories(hakui_presence
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../src
)

target_compile_features(hakui_presence PUBLIC cxx_std_20)
target_link_libraries(hakui_presence PUBLIC hakui_adapter)

if(MSVC)
    target_compile_options(hakui_presence PRIVATE /W4 /permissive-)
else()
    target_compile_options(hakui_presence PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(HAKUI_ENABLE_PRESENCE_SPECS
    "Build the SpiralPresence invariant specification"
    ${BUILD_TESTING}
)

if(HAKUI_ENABLE_PRESENCE_SPECS)
    add_executable(hakui_spiral_presence_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/SpiralPresenceSpec.cpp
    )

    target_compile_features(hakui_spiral_presence_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_spiral_presence_spec PRIVATE hakui_presence)

    if(MSVC)
        target_compile_options(hakui_spiral_presence_spec PRIVATE /W4 /permissive- /UNDEBUG)
    else()
        target_compile_options(hakui_spiral_presence_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
    endif()

    if(BUILD_TESTING)
        add_test(NAME hakui.spiral_presence COMMAND hakui_spiral_presence_spec)
    endif()
endif()
