# PROJECT HAKUI :: READ-ONLY SPIRAL ADAPTER
#
# Semantic observation boundary over HakuiSnapshot. This layer may depend on
# deterministic gameplay truth, but must remain independent from SDL, renderer,
# audio, native platform APIs, model backends, and mutation/action authority.

include(${CMAKE_CURRENT_LIST_DIR}/DependencyFirewall.cmake)

file(GLOB HAKUI_ADAPTER_FIREWALL_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/../src/spiral/hakui/*.hpp"
    "${CMAKE_CURRENT_LIST_DIR}/../src/spiral/hakui/*.cpp"
)

hakui_enforce_first_party_firewall(
    "Hakui Read-Only Adapter"
    ${HAKUI_ADAPTER_FIREWALL_FILES}
)

add_library(hakui_adapter STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/spiral/hakui/HakuiAdapter.cpp
)

target_include_directories(hakui_adapter
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../src
)

target_compile_features(hakui_adapter PUBLIC cxx_std_20)
target_link_libraries(hakui_adapter PUBLIC hakui_gameplay)

if(MSVC)
    target_compile_options(hakui_adapter PRIVATE /W4 /permissive-)
else()
    target_compile_options(hakui_adapter PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(HAKUI_ENABLE_ADAPTER_SPECS
    "Build the Hakui read-only adapter invariant specification"
    ${BUILD_TESTING}
)

if(HAKUI_ENABLE_ADAPTER_SPECS)
    add_executable(hakui_adapter_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/HakuiAdapterSpec.cpp
    )

    target_compile_features(hakui_adapter_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_adapter_spec PRIVATE hakui_adapter)

    if(MSVC)
        target_compile_options(hakui_adapter_spec PRIVATE /W4 /permissive- /UNDEBUG)
    else()
        target_compile_options(hakui_adapter_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
    endif()

    if(BUILD_TESTING)
        add_test(NAME hakui.adapter COMMAND hakui_adapter_spec)
    endif()
endif()
