from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def patch(path: str, old: str, new: str) -> None:
    target = ROOT / path
    text = target.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise RuntimeError(f"L8 patch marker missing in {path}: {old[:120]!r}")
    target.write_text(text.replace(old, new, 1), encoding="utf-8")


# Build graph: presence is a first-party layer above the read-only adapter and
# is linked into the actual native client.
patch(
    "CMakeLists.txt",
    "include(cmake/HakuiAdapter.cmake)\ninclude(cmake/HakuiInput.cmake)",
    "include(cmake/HakuiAdapter.cmake)\ninclude(cmake/HakuiPresence.cmake)\ninclude(cmake/HakuiInput.cmake)",
)
patch(
    "CMakeLists.txt",
    "            hakui_gameplay\n            hakui_adapter\n            hakui_input",
    "            hakui_gameplay\n            hakui_adapter\n            hakui_presence\n            hakui_input",
)

# The Black Room now contains one authored semantic/visual Spiral node. It is a
# terminal affordance, but L8 does not make it an executable interaction.
patch(
    "src/world/BlackRoom.cpp",
    "    // Distant markers make negative space tangible without filling it.\n",
    "    // L8 SPIRAL PRESENCE NODE: physical read-only observation surface.\n"
    "    // The node is authored world truth; conversational cortex binding is L9.\n"
    "    {WorldPrimitiveKind::Terminal, MaterialRole::IndustrialDark,\n"
    "     -6.55f, 1.18f, -5.55f, 3.40f, 2.36f, 0.54f},\n"
    "    {WorldPrimitiveKind::Terminal, MaterialRole::VoidBlack,\n"
    "     -6.55f, 1.55f, -5.24f, 2.96f, 1.48f, 0.08f},\n"
    "    {WorldPrimitiveKind::Signage, MaterialRole::TerminalGreen,\n"
    "     -6.55f, 1.55f, -5.18f, 2.78f, 1.30f, 0.03f},\n"
    "    {WorldPrimitiveKind::Signage, MaterialRole::CrtCyan,\n"
    "     -8.08f, 1.18f, -5.54f, 0.10f, 2.20f, 0.10f},\n"
    "    {WorldPrimitiveKind::Signage, MaterialRole::SignalMagenta,\n"
    "     -5.02f, 1.18f, -5.54f, 0.10f, 2.20f, 0.10f},\n\n"
    "    // Distant markers make negative space tangible without filling it.\n",
)
patch(
    "src/world/BlackRoom.cpp",
    "    {1301, \"SPAWN RECOVERY\",\n",
    "    {1401, \"SPIRAL PRESENCE NODE\",\n"
    "     affordanceMask(WorldAffordance::Terminal),\n"
    "     -8.30f, -4.80f, -0.50f, 3.20f, -6.40f, -4.35f,\n"
    "     {-6.55f, 0.0f, -4.92f, 3.14159265358979323846f},\n"
    "     {-6.55f, 0.0f, -4.10f, 3.14159265358979323846f}},\n"
    "    {1301, \"SPAWN RECOVERY\",\n",
)

# Renderer scene contract carries presentation strings/position only. The
# renderer does not call the adapter itself.
patch(
    "src/render/DebugWorldRenderer.hpp",
    "    hakui::RideableState rideable{};\n",
    "    bool spiralPresenceVisible = false;\n"
    "    bool spiralPresenceLinked = false;\n"
    "    bool spiralPresencePlayerInRange = false;\n"
    "    float spiralNodeX = 0.0f;\n"
    "    float spiralNodeY = 0.0f;\n"
    "    float spiralNodeZ = 0.0f;\n"
    "    std::string_view spiralPresenceHeadline{};\n"
    "    std::string_view spiralPresenceLinkLine{};\n"
    "    std::string_view spiralPresenceWorldLine{};\n"
    "    std::string_view spiralPresencePlayerLine{};\n"
    "    std::string_view spiralPresenceNearbyLine{};\n"
    "    std::string_view spiralPresenceCortexLine{};\n"
    "    hakui::RideableState rideable{};\n",
)

patch(
    "src/render/DebugWorldRenderer.cpp",
    "\n    // Locomotion embodiment is presentation driven by deterministic player\n",
    "\n    // L8 visible Spiral presence. The physical terminal is authored in the\n"
    "    // Black Room; these overlays contain only read-only adapter-derived text.\n"
    "    if (!scene.mannequinLab && scene.spiralPresenceVisible) {\n"
    "        const Uint32 linkPalette = scene.spiralPresenceLinked\n"
    "            ? TerminalGreen\n"
    "            : Danger;\n"
    "        const Uint32 rangePalette = scene.spiralPresencePlayerInRange\n"
    "            ? Cyan\n"
    "            : Midnight;\n"
    "        drawBox(\n"
    "            {scene.spiralNodeX, scene.spiralNodeY + 2.30f, scene.spiralNodeZ - 0.20f},\n"
    "            {0.32f, 0.18f, 0.32f},\n"
    "            linkPalette\n"
    "        );\n"
    "        drawBox(\n"
    "            {scene.spiralNodeX, scene.spiralNodeY + 0.10f, scene.spiralNodeZ + 0.10f},\n"
    "            {2.70f, 0.055f, 0.12f},\n"
    "            rangePalette\n"
    "        );\n"
    "        const Mat4 presenceRoot = translation({\n"
    "            scene.spiralNodeX - 1.30f,\n"
    "            scene.spiralNodeY + 2.05f,\n"
    "            scene.spiralNodeZ + 0.36f\n"
    "        });\n"
    "        drawWorldText(scene.spiralPresenceHeadline, presenceRoot,\n"
    "                      0.014f, 0.022f, 0.010f, 38, linkPalette);\n"
    "        drawWorldText(scene.spiralPresenceLinkLine,\n"
    "                      multiply(presenceRoot, translation({0.0f, -0.24f, 0.0f})),\n"
    "                      0.012f, 0.019f, 0.010f, 40, Cyan);\n"
    "        drawWorldText(scene.spiralPresenceWorldLine,\n"
    "                      multiply(presenceRoot, translation({0.0f, -0.46f, 0.0f})),\n"
    "                      0.010f, 0.017f, 0.010f, 44, Shell);\n"
    "        drawWorldText(scene.spiralPresencePlayerLine,\n"
    "                      multiply(presenceRoot, translation({0.0f, -0.66f, 0.0f})),\n"
    "                      0.010f, 0.017f, 0.010f, 44, Shell);\n"
    "        drawWorldText(scene.spiralPresenceNearbyLine,\n"
    "                      multiply(presenceRoot, translation({0.0f, -0.86f, 0.0f})),\n"
    "                      0.010f, 0.017f, 0.010f, 44, Amber);\n"
    "        drawWorldText(scene.spiralPresenceCortexLine,\n"
    "                      multiply(presenceRoot, translation({0.0f, -1.08f, 0.0f})),\n"
    "                      0.010f, 0.017f, 0.010f, 44, Magenta);\n"
    "    }\n\n"
    "    // Locomotion embodiment is presentation driven by deterministic player\n",
)

# Native application owns exactly one presence bound to exactly one resident
# adapter, maintaining member lifetime order runtime -> adapter -> presence.
patch(
    "src/core/HakuiApp.hpp",
    "#include \"spiral/hakui/HakuiAdapter.hpp\"\n",
    "#include \"spiral/hakui/HakuiAdapter.hpp\"\n"
    "#include \"spiral/hakui/SpiralPresence.hpp\"\n",
)
patch(
    "src/core/HakuiApp.hpp",
    "    hakui::HakuiAdapter hakuiAdapter_{runtime_};\n\n",
    "    hakui::HakuiAdapter hakuiAdapter_{runtime_};\n\n"
    "    // L8 visible embodiment of the read-only HAKUI link. Presence formats\n"
    "    // adapter truth for presentation; CORTEX remains deliberately unbound.\n"
    "    hakui::SpiralPresence spiralPresence_{hakuiAdapter_};\n\n",
)

patch(
    "src/core/HakuiApp.cpp",
    "    SDL_Log(\"[HAKUI] SPIRAL CORE // ONLINE\");\n",
    "    SDL_Log(\"[HAKUI] SPIRAL CORE // ONLINE\");\n"
    "    const hakui::SpiralPresenceView bootPresence = spiralPresence_.view();\n"
    "    SDL_Log(\n"
    "        \"[HAKUI] SPIRAL PRESENCE // %s // CORTEX UNBOUND\",\n"
    "        bootPresence.linked ? \"HAKUI LINK READ ONLY\" : \"LINK OFFLINE\"\n"
    "    );\n",
)
patch(
    "src/core/HakuiApp.cpp",
    "    const std::string_view device =\n        InputResolver::deviceName(inputFrame_.activeDevice);\n",
    "    const std::string_view device =\n        InputResolver::deviceName(inputFrame_.activeDevice);\n"
    "    const hakui::SpiralPresenceView spiralPresenceView = spiralPresence_.view();\n",
)
patch(
    "src/core/HakuiApp.cpp",
    "        const hakui::RoomInteractionFocus focus = blackRoom_.nearestInteraction(player_);\n",
    "        if (spiralPresenceView.playerInRange) {\n"
    "            SDL_snprintf(\n"
    "                title,\n"
    "                sizeof(title),\n"
    "                \"HAKUI v1.01 // SPIRAL PRESENCE // HAKUI LINK %s // CORTEX UNBOUND // WORLD %s // NEARBY %zu // INPUT %.*s\",\n"
    "                spiralPresenceView.linked ? \"READ ONLY\" : \"OFFLINE\",\n"
    "                spiralPresenceView.worldLine.c_str(),\n"
    "                spiralPresenceView.nearbyObjectCount,\n"
    "                static_cast<int>(device.size()), device.data()\n"
    "            );\n"
    "            SDL_SetWindowTitle(window_, title);\n"
    "            return;\n"
    "        }\n"
    "        const hakui::RoomInteractionFocus focus = blackRoom_.nearestInteraction(player_);\n",
)
patch(
    "src/core/HakuiApp.cpp",
    "    HakuiSceneState scene;\n",
    "    const hakui::SpiralPresenceView spiralPresenceView = spiralPresence_.view();\n"
    "    HakuiSceneState scene;\n"
    "    scene.spiralPresenceVisible = true;\n"
    "    scene.spiralPresenceLinked = spiralPresenceView.linked;\n"
    "    scene.spiralPresencePlayerInRange = spiralPresenceView.playerInRange;\n"
    "    scene.spiralNodeX = spiralPresenceView.nodeX;\n"
    "    scene.spiralNodeY = spiralPresenceView.nodeY;\n"
    "    scene.spiralNodeZ = spiralPresenceView.nodeZ;\n"
    "    scene.spiralPresenceHeadline = spiralPresenceView.headline;\n"
    "    scene.spiralPresenceLinkLine = spiralPresenceView.linkLine;\n"
    "    scene.spiralPresenceWorldLine = spiralPresenceView.worldLine;\n"
    "    scene.spiralPresencePlayerLine = spiralPresenceView.playerLine;\n"
    "    scene.spiralPresenceNearbyLine = spiralPresenceView.nearbyLine;\n"
    "    scene.spiralPresenceCortexLine = spiralPresenceView.cortexLine;\n",
)

print("L8 SpiralPresence patch applied")
