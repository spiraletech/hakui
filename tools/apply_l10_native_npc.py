#!/usr/bin/env python3
"""Apply the L10 native-client NPC presentation/integration seams.

The large SDL client and debug renderer remain intentionally outside the
headless gameplay target. Keeping this transformation deterministic lets L10
land the authority contracts first and then integrate the native shell without
hand-editing the historical monolith.
"""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
APP = ROOT / "src/core/HakuiApp.cpp"
RENDERER = ROOT / "src/render/DebugWorldRenderer.cpp"
CORTEX = ROOT / "src/spiral/hakui/SpiralCortexClient.cpp"


def require_replace(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected one marker, found {count}")
    return text.replace(old, new, 1)


app = APP.read_text(encoding="utf-8")

# Route native simulation time through GameRuntime so L10 residents tick from
# the same accepted delta as HakuiWorldState. There are exactly two historical
# paths: chat-input simulation and normal unpaused simulation.
legacy_clock = "world_.elapsedSeconds += dt;"
if legacy_clock in app:
    count = app.count(legacy_clock)
    if count != 2:
        raise RuntimeError(f"HakuiApp clock integration: expected 2 sites, found {count}")
    app = app.replace(legacy_clock, "runtime_.advanceWorld(dt);")
elif app.count("runtime_.advanceWorld(dt);") < 2:
    raise RuntimeError("HakuiApp clock integration markers missing")

app = require_replace(
    app,
    '    SDL_Log("[HAKUI] BLACK ROOM // neon lounge + couch + fusion table + open void");',
    '    SDL_Log("[HAKUI] BLACK ROOM // neon lounge + couch + fusion table + open void");\n'
    '    SDL_Log("[HAKUI] NPC SIM // %zu resident(s) // SAELIS // deterministic // cortex optional",\n'
    '            runtime_.npcs().states().size());',
    "HakuiApp NPC boot status",
)

app = require_replace(
    app,
    "    HakuiSceneState scene;\n    scene.spiralPresenceVisible = true;",
    "    HakuiSceneState scene;\n"
    "    scene.npcs = runtime_.npcs().states();\n"
    "    scene.spiralPresenceVisible = true;",
    "HakuiApp scene resident span",
)

APP.write_text(app, encoding="utf-8")

renderer = RENDERER.read_text(encoding="utf-8")
if "// L10 NPC RESIDENTS // deterministic presentation" not in renderer:
    marker = "    if (scene.sparDummyVisible) {"
    if renderer.count(marker) != 1:
        raise RuntimeError("DebugWorldRenderer NPC insertion marker missing/ambiguous")

    npc_draw = r'''    // L10 NPC RESIDENTS // deterministic presentation
    // Simulation owns the transforms/activities. The debug renderer only draws
    // a lightweight resident shell from the read-only frame span.
    for (const hakui::NpcState& npc : scene.npcs) {
        const bool seated = npc.activity == hakui::NpcActivity::Seated;
        const bool curious =
            npc.mood == hakui::NpcMood::Curious ||
            npc.activity == hakui::NpcActivity::ObservingPlayer ||
            npc.activity == hakui::NpcActivity::ObservingSpiral;

        const Mat4 npcRoot = multiply(
            translation({npc.x, npc.y, npc.z}),
            rotationY(npc.yaw)
        );

        auto npcBox = [&](const Vec3& position,
                          const Vec3& dimensions,
                          Uint32 palette) {
            drawModel(multiply(
                npcRoot,
                multiply(translation(position), scale(dimensions))
            ), palette);
        };

        const Uint32 bodyPalette = curious ? Cyan : Magenta;
        const Uint32 accentPalette = seated ? Amber : Cyan;
        const float pelvisY = seated ? 0.82f : 1.00f;
        const float torsoY = seated ? 1.38f : 1.72f;
        const float headY = seated ? 2.12f : 2.60f;
        const float legY = seated ? 0.42f : 0.70f;
        const float legHeight = seated ? 0.78f : 1.40f;

        npcBox({-0.19f, legY, 0.0f}, {0.25f, legHeight, 0.30f}, Midnight);
        npcBox({ 0.19f, legY, 0.0f}, {0.25f, legHeight, 0.30f}, Midnight);
        npcBox({0.0f, pelvisY, 0.0f}, {0.62f, 0.34f, 0.38f}, bodyPalette);
        npcBox({0.0f, torsoY, 0.0f}, {0.78f, 0.86f, 0.42f}, bodyPalette);
        npcBox({-0.50f, torsoY, 0.0f}, {0.18f, 0.88f, 0.20f}, Shell);
        npcBox({ 0.50f, torsoY, 0.0f}, {0.18f, 0.88f, 0.20f}, Shell);
        npcBox({0.0f, headY - 0.34f, 0.0f}, {0.18f, 0.20f, 0.18f}, accentPalette);
        npcBox({0.0f, headY, 0.0f}, {0.48f, 0.54f, 0.46f}, Shell);

        // Thin cyan/magenta datum above the resident keeps the first NPC easy to
        // locate without pretending L10 already has final nameplate/UI systems.
        npcBox({0.0f, headY + 0.43f, 0.0f}, {0.72f, 0.045f, 0.045f}, accentPalette);
    }

'''
    renderer = renderer.replace(marker, npc_draw + marker, 1)

RENDERER.write_text(renderer, encoding="utf-8")

cortex = CORTEX.read_text(encoding="utf-8")
if 'stream << "npc.count="' not in cortex:
    marker = (
        "    for (std::size_t index = 0; index < "
        "snapshot.world.affordances.size(); ++index) {"
    )
    if cortex.count(marker) != 1:
        raise RuntimeError("SpiralCortexClient NPC context marker missing/ambiguous")

    npc_context = r'''    stream << "npc.count=" << snapshot.npcs.size() << '\n';
    for (std::size_t index = 0; index < snapshot.npcs.size(); ++index) {
        const HakuiNpcSnapshot& npc = snapshot.npcs[index];
        stream << "npc." << index << ".id=" << npc.id << '\n';
        stream << "npc." << index << ".name="
               << sanitize_line(npc.displayName) << '\n';
        stream << "npc." << index << ".position="
               << npc.x << ',' << npc.y << ',' << npc.z << '\n';
        stream << "npc." << index << ".yaw=" << npc.yaw << '\n';
        stream << "npc." << index << ".activity="
               << static_cast<unsigned>(npc.activity) << '\n';
        stream << "npc." << index << ".mood="
               << static_cast<unsigned>(npc.mood) << '\n';
        stream << "npc." << index << ".routine="
               << static_cast<unsigned>(npc.routine) << '\n';
        stream << "npc." << index << ".active_affordance_id="
               << npc.activeAffordanceId << '\n';
        stream << "npc." << index << ".active_seat_id="
               << npc.activeSeatAnchorId << '\n';
        stream << "npc." << index << ".needs="
               << npc.needs.hunger << ','
               << npc.needs.energy << ','
               << npc.needs.social << ','
               << npc.needs.comfort << ','
               << npc.needs.fun << '\n';
    }

'''
    cortex = cortex.replace(marker, npc_context + marker, 1)

CORTEX.write_text(cortex, encoding="utf-8")

print("L10 native NPC integration applied")
