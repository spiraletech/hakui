# PROJECT HAKUI :: SPIRAL CORTEX CLIENT
#
# L9 transport boundary to the real Spiral Ether AI runtime. This layer may
# serialize frozen HAKUI observations and exchange text over loopback, but it
# has no SDL, renderer, gameplay mutation, interaction execution or model code.

include(${CMAKE_CURRENT_LIST_DIR}/DependencyFirewall.cmake)

file(GLOB HAKUI_CORTEX_FIREWALL_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../src/spiral/hakui/SpiralCortexClient.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/spiral/hakui/SpiralCortexClient.cpp"
)

hakui_enforce_first_party_firewall(
    "Hakui Spiral Cortex"
    ${HAKUI_CORTEX_FIREWALL_FILES}
)

add_library(hakui_cortex STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/spiral/hakui/SpiralCortexClient.cpp
)

target_include_directories(hakui_cortex
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../src
)

target_compile_features(hakui_cortex PUBLIC cxx_std_20)
target_link_libraries(hakui_cortex PUBLIC hakui_adapter)
if(WIN32)
    target_link_libraries(hakui_cortex PUBLIC ws2_32)
endif()

if(MSVC)
    target_compile_options(hakui_cortex PRIVATE /W4 /permissive-)
else()
    target_compile_options(hakui_cortex PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(HAKUI_ENABLE_CORTEX_SPECS
    "Build the Spiral cortex invariant specification"
    ${BUILD_TESTING}
)

if(HAKUI_ENABLE_CORTEX_SPECS)
    add_executable(hakui_spiral_cortex_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/SpiralCortexSpec.cpp
    )

    target_compile_features(hakui_spiral_cortex_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_spiral_cortex_spec PRIVATE hakui_cortex)

    if(MSVC)
        target_compile_options(hakui_spiral_cortex_spec PRIVATE /W4 /permissive- /UNDEBUG)
    else()
        target_compile_options(hakui_spiral_cortex_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
    endif()

    if(BUILD_TESTING)
        add_test(NAME hakui.spiral_cortex COMMAND hakui_spiral_cortex_spec)
    endif()
endif()
