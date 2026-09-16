# Pin HOME's L25 authority implementation. Offline builds may supply the exact
# checkout using -DFETCHCONTENT_SOURCE_DIR_SPIRAL_HOME=/path/to/SPIRAL-OS-HOME.
FetchContent_Declare(spiral_home
    GIT_REPOSITORY https://github.com/spiraletech/SPIRAL-OS-HOME.git
    GIT_TAG 2a703f0b5a983569253b339454fd36dc7f22ae3c
)
set(HOME_BUILD_TESTS ${BUILD_TESTING} CACHE BOOL "Build HOME tests")
FetchContent_MakeAvailable(spiral_home)
add_library(hakui_home STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/home/HomeBridge.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/home/HomeSession.cpp)
target_include_directories(hakui_home PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../src)
target_link_libraries(hakui_home PUBLIC HOME::Core)
target_compile_features(hakui_home PUBLIC cxx_std_20)
if(MSVC)
    target_compile_options(hakui_home PRIVATE /W4 /permissive-)
else()
    target_compile_options(hakui_home PRIVATE -Wall -Wextra -Wpedantic)
endif()
if(BUILD_TESTING)
    add_executable(hakui_home_spec ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/HomeBridgeSpec.cpp)
    target_link_libraries(hakui_home_spec PRIVATE hakui_home hakui_gameplay)
    if(MSVC)
        target_compile_options(hakui_home_spec PRIVATE /UNDEBUG)
    else()
        target_compile_options(hakui_home_spec PRIVATE -UNDEBUG)
    endif()
    add_test(NAME hakui.home_bridge COMMAND hakui_home_spec)
    add_test(NAME hakui.home_restart COMMAND ${CMAKE_COMMAND}
        -DSPEC=$<TARGET_FILE:hakui_home_spec>
        -DSAVE=${CMAKE_CURRENT_BINARY_DIR}/home-restart.snapshot
        -P ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/HomeRestart.cmake)
endif()
