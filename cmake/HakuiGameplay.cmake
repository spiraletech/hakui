# PROJECT HAKUI :: GAMEPLAY LAYER
#
# Deterministic first-party gameplay rules. This target owns simulation logic,
# not platform input, rendering, audio, model inference or persistence.

include(${CMAKE_CURRENT_LIST_DIR}/DependencyFirewall.cmake)

file(GLOB HAKUI_GAMEPLAY_FIREWALL_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../src/player/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/player/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/npc/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/npc/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/action/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/action/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/navigation/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/navigation/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/witness/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/witness/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/world/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/world/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/core/GameRuntime.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/core/HakuiSnapshot.hpp"
)

hakui_enforce_first_party_firewall(
    "Hakui Gameplay"
    ${HAKUI_GAMEPLAY_FIREWALL_FILES}
)

add_library(hakui_gameplay STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/action/HakuiActionGate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/navigation/HakuiNavigation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/player/PlayerMovementController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/player/RideableMovementController.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/npc/NpcManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/world/BlackRoom.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/core/GameRuntime.hpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/core/HakuiSnapshot.hpp
)

target_include_directories(hakui_gameplay
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../src
)

target_compile_features(hakui_gameplay PUBLIC cxx_std_20)
target_link_libraries(hakui_gameplay PUBLIC hakui_witness)

if(MSVC)
    target_compile_options(hakui_gameplay PRIVATE /W4 /permissive-)
else()
    target_compile_options(hakui_gameplay PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(HAKUI_ENABLE_GAMEPLAY_SPECS
    "Build the Hakui deterministic-gameplay specification"
    ${BUILD_TESTING}
)

if(HAKUI_ENABLE_GAMEPLAY_SPECS)
    add_executable(hakui_gameplay_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/GameplayMovementSpec.cpp
    )

    add_executable(hakui_rideable_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/RideableMovementSpec.cpp
    )

    add_executable(hakui_world_regression_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/WorldRegressionSpec.cpp
    )

    add_executable(hakui_world_state_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/HakuiWorldStateSpec.cpp
    )

    add_executable(hakui_game_runtime_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/GameRuntimeSpec.cpp
    )

    add_executable(hakui_snapshot_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/HakuiSnapshotSpec.cpp
    )

    add_executable(hakui_npc_simulation_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/NpcSimulationSpec.cpp
    )

    add_executable(hakui_action_gate_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/HakuiActionGateSpec.cpp
    )

    add_executable(hakui_navigation_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/HakuiNavigationSpec.cpp
    )

    target_compile_features(hakui_gameplay_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_gameplay_spec PRIVATE hakui_gameplay)
    target_compile_features(hakui_rideable_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_rideable_spec PRIVATE hakui_gameplay)
    target_compile_features(hakui_world_regression_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_world_regression_spec PRIVATE hakui_gameplay)
    target_compile_features(hakui_world_state_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_world_state_spec PRIVATE hakui_gameplay)
    target_compile_features(hakui_game_runtime_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_game_runtime_spec PRIVATE hakui_gameplay)
    target_compile_features(hakui_snapshot_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_snapshot_spec PRIVATE hakui_gameplay)
    target_compile_features(hakui_npc_simulation_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_npc_simulation_spec PRIVATE hakui_gameplay)
    target_compile_features(hakui_action_gate_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_action_gate_spec PRIVATE hakui_gameplay)
    target_compile_features(hakui_navigation_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_navigation_spec PRIVATE hakui_gameplay)

    if(MSVC)
        target_compile_options(hakui_gameplay_spec PRIVATE /W4 /permissive- /UNDEBUG)
        target_compile_options(hakui_rideable_spec PRIVATE /W4 /permissive- /UNDEBUG)
        target_compile_options(hakui_world_regression_spec PRIVATE /W4 /permissive- /UNDEBUG)
        target_compile_options(hakui_world_state_spec PRIVATE /W4 /permissive- /UNDEBUG)
        target_compile_options(hakui_game_runtime_spec PRIVATE /W4 /permissive- /UNDEBUG)
        target_compile_options(hakui_snapshot_spec PRIVATE /W4 /permissive- /UNDEBUG)
        target_compile_options(hakui_npc_simulation_spec PRIVATE /W4 /permissive- /UNDEBUG)
        target_compile_options(hakui_action_gate_spec PRIVATE /W4 /permissive- /UNDEBUG)
        target_compile_options(hakui_navigation_spec PRIVATE /W4 /permissive- /UNDEBUG)
    else()
        target_compile_options(hakui_gameplay_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
        target_compile_options(hakui_rideable_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
        target_compile_options(hakui_world_regression_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
        target_compile_options(hakui_world_state_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
        target_compile_options(hakui_game_runtime_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
        target_compile_options(hakui_snapshot_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
        target_compile_options(hakui_npc_simulation_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
        target_compile_options(hakui_action_gate_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
        target_compile_options(hakui_navigation_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
    endif()

    if(BUILD_TESTING)
        add_test(NAME hakui.gameplay_movement COMMAND hakui_gameplay_spec)
        add_test(NAME hakui.rideable_movement COMMAND hakui_rideable_spec)
        add_test(NAME hakui.world_regression COMMAND hakui_world_regression_spec)
        add_test(NAME hakui.world_state COMMAND hakui_world_state_spec)
        add_test(NAME hakui.game_runtime COMMAND hakui_game_runtime_spec)
        add_test(NAME hakui.snapshot COMMAND hakui_snapshot_spec)
        add_test(NAME hakui.npc_simulation COMMAND hakui_npc_simulation_spec)
        add_test(NAME hakui.action_gate COMMAND hakui_action_gate_spec)
        add_test(NAME hakui.navigation COMMAND hakui_navigation_spec)
    endif()
endif()
