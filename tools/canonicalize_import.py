from pathlib import Path


def replace_once(path: Path, old: str, new: str, label: str) -> None:
    text = path.read_text(encoding="utf-8")
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected one match in {path}, found {count}")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")
    print(f"[HAKUI CANON] {label}")


avatar_cmake = Path("cmake/HakuiAvatarRig.cmake")

replace_once(
    avatar_cmake,
    "add_library(hakui_avatar_rig STATIC\n"
    "    ${CMAKE_CURRENT_LIST_DIR}/../src/avatar/AvatarGroundContact.cpp\n",
    "add_library(hakui_avatar_rig STATIC\n"
    "    ${CMAKE_CURRENT_LIST_DIR}/../src/avatar/BodyProfile.cpp\n"
    "    ${CMAKE_CURRENT_LIST_DIR}/../src/avatar/BodyProfileController.cpp\n"
    "    ${CMAKE_CURRENT_LIST_DIR}/../src/avatar/AvatarGroundContact.cpp\n",
    "compile runtime body profiles with the canonical avatar rig",
)

replace_once(
    avatar_cmake,
    "    if(BUILD_TESTING)\n"
    "        add_test(NAME hakui.avatar_rig COMMAND hakui_avatar_rig_spec)\n"
    "    endif()\n"
    "endif()\n",
    "    if(BUILD_TESTING)\n"
    "        add_test(NAME hakui.avatar_rig COMMAND hakui_avatar_rig_spec)\n"
    "    endif()\n"
    "endif()\n\n"
    "add_executable(hakui_body_profile_spec\n"
    "    ${CMAKE_CURRENT_LIST_DIR}/../tests/BodyProfileSpec.cpp\n"
    ")\n"
    "target_compile_features(hakui_body_profile_spec PRIVATE cxx_std_20)\n"
    "target_link_libraries(hakui_body_profile_spec PRIVATE hakui_avatar_rig)\n"
    "add_executable(hakui_body_profile_controller_spec\n"
    "    ${CMAKE_CURRENT_LIST_DIR}/../tests/hakui/BodyProfileControllerSpec.cpp\n"
    ")\n"
    "target_compile_features(hakui_body_profile_controller_spec PRIVATE cxx_std_20)\n"
    "target_link_libraries(hakui_body_profile_controller_spec PRIVATE hakui_avatar_rig)\n"
    "if(MSVC)\n"
    "    target_compile_options(hakui_body_profile_spec PRIVATE /W4 /permissive- /UNDEBUG)\n"
    "    target_compile_options(hakui_body_profile_controller_spec PRIVATE /W4 /permissive- /UNDEBUG)\n"
    "else()\n"
    "    target_compile_options(hakui_body_profile_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)\n"
    "    target_compile_options(hakui_body_profile_controller_spec PRIVATE -Wall -Wextra -Wpedantic -UNDEBUG)\n"
    "endif()\n"
    "if(BUILD_TESTING)\n"
    "    add_test(NAME hakui.body_profiles COMMAND hakui_body_profile_spec)\n"
    "    add_test(NAME hakui.body_profile_controller COMMAND hakui_body_profile_controller_spec)\n"
    "endif()\n",
    "register runtime body-profile contracts",
)

marker = Path("docs/CANONICAL_SOURCE.md")
marker.write_text(
    "# Canonical source snapshot\n\n"
    "The initial gameplay snapshot in this repository is imported from "
    "`spiraletech/spiral-ether-tech` commit "
    "`c03da5a02b3ac288186c6468b08b5ae2e8ce55a9` "
    "(`gpt/hakui-female-player-v1.01`).\n\n"
    "That commit is a strict descendant of the male v1.01 skate-embodiment "
    "head `d967d3e1002b69cbc81f9b75c97f1a9d139b1c44`, so it carries the full "
    "v1.01 gameplay lineage plus later mannequin/female-shell work. The "
    "canonical HAKUI repository converts those build-time shell forks into "
    "runtime body profiles while preserving one gameplay authority.\n",
    encoding="utf-8",
)

print("[HAKUI CANON] import canonicalization complete")
