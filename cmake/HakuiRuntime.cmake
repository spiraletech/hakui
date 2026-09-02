# PROJECT HAKUI :: GAME RUNTIME
#
# Deterministic live-game ownership boundary. This layer composes first-party
# world/player/gameplay state without depending on SDL, rendering, audio, or
# platform APIs.

include(${CMAKE_CURRENT_LIST_DIR}/DependencyFirewall.cmake)

hakui_enforce_first_party_firewall(
    "Hakui Runtime"
    ${CMAKE_CURRENT_LIST_DIR}/../src/core/GameRuntime.hpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/core/GameRuntime.cpp
)

add_library(hakui_runtime STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/core/GameRuntime.cpp
)

target_include_directories(hakui_runtime
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../src
)

target_compile_features(hakui_runtime PUBLIC cxx_std_20)
target_link_libraries(hakui_runtime
    PUBLIC
        hakui_gameplay
        hakui_combat
        hakui_social
)

if(MSVC)
    target_compile_options(hakui_runtime PRIVATE /W4 /permissive-)
else()
    target_compile_options(hakui_runtime PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(HAKUI_ENABLE_RUNTIME_SPECS
    "Build the Hakui GameRuntime ownership specification"
    ${BUILD_TESTING}
)

if(HAKUI_ENABLE_RUNTIME_SPECS)
    add_executable(hakui_runtime_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/GameRuntimeSpec.cpp
    )

    target_compile_features(hakui_runtime_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_runtime_spec PRIVATE hakui_runtime)

    if(MSVC)
        target_compile_options(hakui_runtime_spec PRIVATE /W4 /permissive- /UNDEBUG)
    else()
        target_compile_options(hakui_runtime_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
    endif()

    if(BUILD_TESTING)
        add_test(NAME hakui.runtime COMMAND hakui_runtime_spec)
    endif()
endif()
