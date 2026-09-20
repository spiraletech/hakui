# PROJECT HAKUI :: BODY POSE
#
# HAKUI-owned body solution stage. This consumes deterministic player/ride
# state plus body dimensions and emits solved joint/root targets. Renderers
# may consume the result but must not mutate or author simulation truth.

include(${CMAKE_CURRENT_LIST_DIR}/DependencyFirewall.cmake)

file(GLOB HAKUI_BODY_POSE_FIREWALL_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../src/body/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/body/*.cpp"
)

hakui_enforce_first_party_firewall(
    "Hakui Body Pose"
    ${HAKUI_BODY_POSE_FIREWALL_FILES}
)

add_library(hakui_body_pose STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/body/BodyPoseSolver.cpp
)

target_include_directories(hakui_body_pose
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../src
)

target_compile_features(hakui_body_pose PUBLIC cxx_std_20)
target_link_libraries(hakui_body_pose
    PUBLIC
        hakui_avatar_rig
        hakui_gameplay
)

if(MSVC)
    target_compile_options(hakui_body_pose PRIVATE /W4 /permissive-)
else()
    target_compile_options(hakui_body_pose PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(HAKUI_ENABLE_BODY_POSE_SPECS
    "Build the HAKUI body-pose solver specification"
    ${BUILD_TESTING}
)

if(HAKUI_ENABLE_BODY_POSE_SPECS)
    add_executable(hakui_body_pose_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/BodyPoseSolverSpec.cpp
    )
    target_compile_features(hakui_body_pose_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_body_pose_spec PRIVATE hakui_body_pose)

    if(MSVC)
        target_compile_options(hakui_body_pose_spec PRIVATE /W4 /permissive- /UNDEBUG)
    else()
        target_compile_options(hakui_body_pose_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
    endif()

    if(BUILD_TESTING)
        add_test(NAME hakui.body_pose COMMAND hakui_body_pose_spec)
    endif()
endif()
