include(${CMAKE_CURRENT_LIST_DIR}/DependencyFirewall.cmake)

hakui_enforce_first_party_firewall("Hakui L13 Intent Proposals"
    ${CMAKE_CURRENT_LIST_DIR}/../src/intent/HakuiIntentProposal.hpp
    ${CMAKE_CURRENT_LIST_DIR}/../src/intent/HakuiIntentProposal.cpp)

add_library(hakui_intent STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../src/intent/HakuiIntentProposal.cpp)
target_include_directories(hakui_intent PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../src)
target_compile_features(hakui_intent PUBLIC cxx_std_20)
if(MSVC)
    target_compile_options(hakui_intent PRIVATE /W4 /permissive-)
else()
    target_compile_options(hakui_intent PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    add_executable(hakui_intent_spec
        ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/HakuiIntentProposalSpec.cpp)
    target_link_libraries(hakui_intent_spec PRIVATE hakui_intent)
    if(MSVC)
        target_compile_options(hakui_intent_spec PRIVATE /W4 /permissive- /UNDEBUG)
    else()
        target_compile_options(hakui_intent_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)
    endif()
    add_test(NAME hakui.intent_proposals COMMAND hakui_intent_spec)
endif()
