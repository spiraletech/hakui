cmake_minimum_required(VERSION 3.25)

project(hakui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(hakui_avatar_profile STATIC
    src/avatar/BodyProfile.cpp
)

target_include_directories(hakui_avatar_profile
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_features(hakui_avatar_profile PUBLIC cxx_std_20)

include(CTest)
if(BUILD_TESTING)
    add_executable(hakui_body_profile_spec
        tests/BodyProfileSpec.cpp
    )
    target_link_libraries(hakui_body_profile_spec PRIVATE hakui_avatar_profile)
    add_test(NAME hakui_body_profile_spec COMMAND hakui_body_profile_spec)
endif()
