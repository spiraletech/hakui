# PROJECT HAKUI :: L12 WITNESS
#
# Bounded, deterministic self-observation records. This layer owns no gameplay,
# platform, rendering, network, model, or persistence authority.

include(${CMAKE_CURRENT_LIST_DIR}/DependencyFirewall.cmake)

hakui_enforce_first_party_firewall(
    "Hakui Witness"
    ${CMAKE_CURRENT_LIST_DIR}/../src/witness/HakuiWitness.hpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/witness/HakuiWitness.cpp
)

add_library(hakui_witness STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/witness/HakuiWitness.cpp
)

target_include_directories(hakui_witness
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../src
)

target_compile_features(hakui_witness PUBLIC cxx_std_20)

if(MSVC)
    target_compile_options(hakui_witness PRIVATE /W4 /permissive-)
else()
    target_compile_options(hakui_witness PRIVATE -Wall -Wextra -Wpedantic)
endif()

option(HAKUI_ENABLE_WITNESS_SPECS
    "Build the HAKUI L12 witness specification"
    ${BUILD_TESTING}
)

if(HAKUI_ENABLE_WITNESS_SPECS)
    add_executable(hakui_witness_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/HakuiWitnessSpec.cpp
    )
    target_compile_features(hakui_witness_spec PRIVATE cxx_std_20)
    target_link_libraries(hakui_witness_spec PRIVATE hakui_witness)
    if(MSVC)
        target_compile_options(hakui_witness_spec PRIVATE /W4 /permissive- /UNDEBUG)
    else()
        target_compile_options(hakui_witness_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
    endif()
    if(BUILD_TESTING)
        add_test(NAME hakui.witness COMMAND hakui_witness_spec)
    endif()
endif()
