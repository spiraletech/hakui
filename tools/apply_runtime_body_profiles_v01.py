from pathlib import Path


def replace_once(path: Path, old: str, new: str, label: str) -> None:
    text = path.read_text(encoding="utf-8")
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match in {path}, found {count}")
    path.write_text(text.replace(old, new, 1), encoding="utf-8")
    print(f"[HAKUI BODY v0.1] {label}")


renderer = Path("src/render/DebugWorldRenderer.cpp")
app = Path("src/core/HakuiApp.cpp")
marker = Path("docs/RUNTIME_BODY_PROFILES.md")

# -----------------------------------------------------------------------------
# Renderer: resolve one effective presentation profile. Mannequin Lab keeps its
# dedicated male/female selector; normal gameplay reads the runtime profile
# owned by HakuiApp. Both paths consume the same immutable BodyProfile data.
# -----------------------------------------------------------------------------
replace_once(
    renderer,
    "    auto leg = [&](float side, float angle, float kneeAngle) {\n",
    "    const hakui::avatar::BodyProfile& activeBody = scene.mannequinLab\n"
    "        ? (scene.mannequinFemale\n"
    "            ? hakui::avatar::femaleBodyProfile()\n"
    "            : hakui::avatar::maleBodyProfile())\n"
    "        : hakui::avatar::bodyProfile(scene.playerBodyProfile);\n"
    "    const auto bodyVec = [](const hakui::avatar::Vec3f& value) {\n"
    "        return Vec3{value.x, value.y, value.z};\n"
    "    };\n\n"
    "    auto leg = [&](float side, float angle, float kneeAngle) {\n",
    "resolve effective body profile",
)

replace_once(
    renderer,
    "        drawModel(multiply(upper,\n"
    "            multiply(translation({0.0f, -0.38f, 0.0f}),\n"
    "                     scale({0.30f, 0.76f, 0.34f}))), Shell);\n\n"
    "        const Mat4 lower = multiply(\n"
    "            upper,\n"
    "            multiply(translation({0.0f, -0.76f, 0.0f}), rotationX(kneeAngle))\n"
    "        );\n"
    "        drawModel(multiply(lower,\n"
    "            multiply(translation({0.0f, -0.37f, 0.0f}),\n"
    "                     scale({0.28f, 0.74f, 0.30f}))), Shell);\n"
    "        drawModel(multiply(lower,\n"
    "            multiply(translation({0.0f, -0.76f, 0.14f}),\n"
    "                     scale({0.32f, 0.16f, 0.58f}))), Cyan);\n",
    "        drawModel(multiply(upper,\n"
    "            multiply(translation({0.0f, -0.38f, 0.0f}),\n"
    "                     scale({activeBody.thighRadius, 0.76f, activeBody.thighRadius}))), Shell);\n\n"
    "        const Mat4 lower = multiply(\n"
    "            upper,\n"
    "            multiply(translation({0.0f, -0.76f, 0.0f}), rotationX(kneeAngle))\n"
    "        );\n"
    "        drawModel(multiply(lower,\n"
    "            multiply(translation({0.0f, -0.37f, 0.0f}),\n"
    "                     scale({activeBody.calfRadius, 0.74f, activeBody.calfRadius}))), Shell);\n"
    "        drawModel(multiply(lower,\n"
    "            multiply(translation({0.0f, -0.76f, 0.14f}),\n"
    "                     scale(bodyVec(activeBody.footSize)))), Cyan);\n",
    "use profile geometry for simple legs",
)

replace_once(
    renderer,
    "        hingedBox(\n"
    "            {side * 0.60f, 2.12f, 0.0f},\n"
    "            angle,\n"
    "            {0.0f, -0.50f, 0.0f},\n"
    "            {0.24f, 1.00f, 0.28f}\n"
    "        );\n",
    "        hingedBox(\n"
    "            {side * activeBody.shoulderHalfWidth, activeBody.shoulderHeight, 0.0f},\n"
    "            angle,\n"
    "            {0.0f, -0.50f, 0.0f},\n"
    "            {activeBody.upperArmRadius, 1.00f, activeBody.upperArmRadius}\n"
    "        );\n",
    "use profile geometry for simple arms",
)

replace_once(
    renderer,
    "        contactSegment(\n"
    "            hip, knee,\n"
    "            scene.mannequinLab\n"
    "                ? (scene.mannequinFemale ? 0.205f : 0.188f)\n"
    "                : 0.27f\n"
    "        );\n"
    "        contactSegment(\n"
    "            knee, target,\n"
    "            scene.mannequinLab\n"
    "                ? (scene.mannequinFemale ? 0.132f : 0.145f)\n"
    "                : 0.25f\n"
    "        );\n",
    "        contactSegment(hip, knee, activeBody.thighRadius);\n"
    "        contactSegment(knee, target, activeBody.calfRadius);\n",
    "use profile radii for contact legs",
)

replace_once(
    renderer,
    "            scene.mannequinLab\n"
    "                ? (scene.mannequinFemale\n"
    "                    ? Vec3{0.185f, 0.082f, 0.330f}\n"
    "                    : Vec3{0.205f, 0.095f, 0.360f})\n"
    "                : Vec3{0.32f, 0.14f, 0.50f},\n",
    "            bodyVec(activeBody.footSize),\n",
    "use profile feet for contact legs",
)

replace_once(
    renderer,
    "        const Vec3 shoulder = rotateYawPoint(\n"
    "            scene.mannequinLab\n"
    "                ? (scene.mannequinFemale\n"
    "                    ? Vec3{side * 0.46f, 2.02f, 0.0f}\n"
    "                    : Vec3{side * 0.52f, 2.04f, 0.0f})\n"
    "                : Vec3{side * 0.60f, 2.12f, 0.0f},\n"
    "            shoulderYaw\n"
    "        );\n"
    "        const float elbowOut = scene.mannequinLab\n"
    "            ? (scene.mannequinFemale ? 0.060f : 0.075f)\n"
    "            : 0.12f;\n",
    "        const Vec3 shoulder = rotateYawPoint(\n"
    "            {side * activeBody.shoulderHalfWidth, activeBody.shoulderHeight, 0.0f},\n"
    "            shoulderYaw\n"
    "        );\n"
    "        const float elbowOut = activeBody.elbowOut;\n",
    "use profile shoulder and elbow line",
)

replace_once(
    renderer,
    "        contactSegment(\n"
    "            shoulder, elbow,\n"
    "            scene.mannequinLab\n"
    "                ? (scene.mannequinFemale ? 0.132f : 0.145f)\n"
    "                : 0.23f\n"
    "        );\n"
    "        contactSegment(\n"
    "            elbow, target,\n"
    "            scene.mannequinLab\n"
    "                ? (scene.mannequinFemale ? 0.094f : 0.112f)\n"
    "                : 0.21f\n"
    "        );\n",
    "        contactSegment(shoulder, elbow, activeBody.upperArmRadius);\n"
    "        contactSegment(elbow, target, activeBody.forearmRadius);\n",
    "use profile radii for contact arms",
)

replace_once(
    renderer,
    "            scene.mannequinLab\n"
    "                ? (scene.mannequinFemale\n"
    "                    ? Vec3{0.115f, 0.095f, 0.135f}\n"
    "                    : Vec3{0.135f, 0.115f, 0.150f})\n"
    "                : Vec3{0.24f, 0.20f, 0.24f},\n",
    "            bodyVec(activeBody.handSize),\n",
    "use profile hands for contact arms",
)

replace_once(
    renderer,
    "        scene.mannequinLab\n"
    "            ? (scene.mannequinFemale\n"
    "                ? Vec3{0.64f, 0.22f, 0.36f}\n"
    "                : Vec3{0.54f, 0.20f, 0.32f})\n"
    "            : Vec3{0.74f, 0.30f, 0.44f},\n",
    "        bodyVec(activeBody.pelvisSize),\n",
    "use profile pelvis in gameplay and lab",
)

replace_once(
    renderer,
    "    if (scene.mannequinLab) {\n"
    "        // Upper pelvis/waist bridge: stacked geometry gives the lab body\n"
    "        // a simple wedge transition instead of torso -> belt -> legs.\n"
    "        orientedLocalBox(\n"
    "            {0.0f,\n"
    "             1.32f - scene.rideable.body.preloadPoseWeight * 0.16f -\n"
    "                 scene.rideable.body.landingCompression * 0.13f,\n"
    "             0.0f},\n"
    "            scene.mannequinFemale\n"
    "                ? Vec3{0.40f, 0.13f, 0.29f}\n"
    "                : Vec3{0.46f, 0.13f, 0.30f},\n"
    "            scene.rideable.body.pelvisYawRelativeToBoard,\n"
    "            Midnight\n"
    "        );\n"
    "    }\n",
    "    // Shared pelvis-to-waist bridge. This is presentation only and follows\n"
    "    // the active body profile without changing gameplay contact targets.\n"
    "    orientedLocalBox(\n"
    "        {0.0f,\n"
    "         1.32f - scene.rideable.body.preloadPoseWeight * 0.16f -\n"
    "             scene.rideable.body.landingCompression * 0.13f,\n"
    "         0.0f},\n"
    "        bodyVec(activeBody.waistBridgeSize),\n"
    "        scene.rideable.body.pelvisYawRelativeToBoard,\n"
    "        Midnight\n"
    "    );\n",
    "promote waist bridge to gameplay",
)

replace_once(
    renderer,
    "                        scale({\n"
    "                            scene.mannequinLab\n"
    "                                ? (scene.mannequinFemale ? 0.54f : 0.58f)\n"
    "                                : 0.92f,\n"
    "                            scene.mannequinLab\n"
    "                                ? (scene.mannequinFemale ? 0.84f : 0.86f)\n"
    "                                : 0.94f,\n"
    "                            scene.mannequinLab\n"
    "                                ? (scene.mannequinFemale ? 0.30f : 0.31f)\n"
    "                                : 0.48f\n"
    "                        })\n",
    "                        scale(bodyVec(activeBody.torsoFrame))\n",
    "use profile torso frame",
)

replace_once(
    renderer,
    "    if (scene.mannequinLab) {\n"
    "        drawModel(\n"
    "            multiply(\n"
    "                torso,\n"
    "                multiply(\n"
    "                    translation({0.0f, 0.18f, 0.0f}),\n"
    "                    scale({\n"
    "                        scene.mannequinFemale ? 1.16f : 1.24f,\n"
    "                        0.58f,\n"
    "                        scene.mannequinFemale ? 1.08f : 1.10f\n"
    "                    })\n"
    "                )\n"
    "            ),\n"
    "            torsoPalette\n"
    "        );\n"
    "        drawModel(\n"
    "            multiply(\n"
    "                torso,\n"
    "                multiply(\n"
    "                    translation({0.0f, -0.28f, 0.0f}),\n"
    "                    scale({\n"
    "                        scene.mannequinFemale ? 0.72f : 0.86f,\n"
    "                        0.38f,\n"
    "                        scene.mannequinFemale ? 0.92f : 0.96f\n"
    "                    })\n"
    "                )\n"
    "            ),\n"
    "            torsoPalette\n"
    "        );\n"
    "    } else {\n"
    "        drawModel(torso, torsoPalette);\n"
    "    }\n",
    "    drawModel(\n"
    "        multiply(\n"
    "            torso,\n"
    "            multiply(\n"
    "                translation({0.0f, 0.18f, 0.0f}),\n"
    "                scale({activeBody.ribcageWidthScale, 0.58f, activeBody.ribcageDepthScale})\n"
    "            )\n"
    "        ),\n"
    "        torsoPalette\n"
    "    );\n"
    "    drawModel(\n"
    "        multiply(\n"
    "            torso,\n"
    "            multiply(\n"
    "                translation({0.0f, -0.28f, 0.0f}),\n"
    "                scale({activeBody.waistWidthScale, 0.38f, activeBody.waistDepthScale})\n"
    "            )\n"
    "        ),\n"
    "        torsoPalette\n"
    "    );\n",
    "promote tapered torso to gameplay",
)

replace_once(
    renderer,
    "    if (scene.mannequinLab) {\n"
    "        const float torsoYaw = scene.rideable.body.torsoYawRelativeToBoard;\n"
    "        const Vec3 clavicleCenter = rotateYawPoint({0.0f, 2.12f, 0.0f}, torsoYaw);\n"
    "        const float clavicleSpread = scene.mannequinFemale ? 0.46f : 0.52f;\n"
    "        const float clavicleY = scene.mannequinFemale ? 2.02f : 2.04f;\n"
    "        const Vec3 leftShoulder = rotateYawPoint(\n"
    "            {-clavicleSpread, clavicleY, 0.0f}, torsoYaw\n"
    "        );\n"
    "        const Vec3 rightShoulder = rotateYawPoint(\n"
    "            {clavicleSpread, clavicleY, 0.0f}, torsoYaw\n"
    "        );\n"
    "        const float clavicleWidth = scene.mannequinFemale ? 0.082f : 0.095f;\n"
    "        contactSegment(clavicleCenter, leftShoulder, clavicleWidth, Shell);\n"
    "        contactSegment(clavicleCenter, rightShoulder, clavicleWidth, Shell);\n"
    "    }\n",
    "    const float torsoYaw = scene.rideable.body.torsoYawRelativeToBoard;\n"
    "    const Vec3 clavicleCenter = rotateYawPoint({0.0f, 2.12f, 0.0f}, torsoYaw);\n"
    "    const Vec3 leftShoulder = rotateYawPoint(\n"
    "        {-activeBody.shoulderHalfWidth, activeBody.shoulderHeight, 0.0f}, torsoYaw\n"
    "    );\n"
    "    const Vec3 rightShoulder = rotateYawPoint(\n"
    "        {activeBody.shoulderHalfWidth, activeBody.shoulderHeight, 0.0f}, torsoYaw\n"
    "    );\n"
    "    contactSegment(clavicleCenter, leftShoulder, activeBody.clavicleRadius, Shell);\n"
    "    contactSegment(clavicleCenter, rightShoulder, activeBody.clavicleRadius, Shell);\n",
    "promote profile clavicles to gameplay",
)

replace_once(
    renderer,
    "        scene.mannequinLab\n"
    "            ? (scene.mannequinFemale\n"
    "                ? Vec3{0.125f, 0.190f, 0.125f}\n"
    "                : Vec3{0.145f, 0.205f, 0.145f})\n"
    "            : Vec3{0.22f, 0.18f, 0.22f},\n",
    "        bodyVec(activeBody.neckSize),\n",
    "use profile neck in gameplay and lab",
)

replace_once(
    renderer,
    "        scene.mannequinLab\n"
    "            ? (scene.mannequinFemale\n"
    "                ? Vec3{0.39f, 0.47f, 0.37f}\n"
    "                : Vec3{0.42f, 0.50f, 0.40f})\n"
    "            : Vec3{0.56f, 0.58f, 0.52f},\n",
    "        bodyVec(activeBody.headSize),\n",
    "use profile head in gameplay and lab",
)

# -----------------------------------------------------------------------------
# HakuiApp: one runtime owner, optional startup profile, F6 hot switch, StateStore
# publication, and scene propagation. No locomotion/combat/physics state changes.
# -----------------------------------------------------------------------------
replace_once(
    app,
    "    initSpiralCore();\n",
    "    if (const char* requestedBody = SDL_getenv(\"HAKUI_BODY_PROFILE\");\n"
    "        requestedBody && requestedBody[0] != '\\0') {\n"
    "        if (!bodyProfile_.setFromName(requestedBody)) {\n"
    "            SDL_LogWarn(\n"
    "                SDL_LOG_CATEGORY_APPLICATION,\n"
    "                \"[HAKUI] unknown HAKUI_BODY_PROFILE '%s' // using %.*s\",\n"
    "                requestedBody,\n"
    "                static_cast<int>(bodyProfile_.active().name.size()),\n"
    "                bodyProfile_.active().name.data()\n"
    "            );\n"
    "        }\n"
    "    }\n\n"
    "    initSpiralCore();\n",
    "add startup body-profile selection",
)

replace_once(
    app,
    "    SDL_Log(\"[HAKUI] avatar skeleton // %zu bones loaded\", avatarSkeleton_.boneCount());\n",
    "    SDL_Log(\"[HAKUI] avatar skeleton // %zu bones loaded\", avatarSkeleton_.boneCount());\n"
    "    SDL_Log(\n"
    "        \"[HAKUI] body profile // %.*s // runtime-selectable\",\n"
    "        static_cast<int>(bodyProfile_.active().name.size()),\n"
    "        bodyProfile_.active().name.data()\n"
    "    );\n",
    "log active body profile at boot",
)

replace_once(
    app,
    "    SDL_Log(\"[HAKUI] controls // WASD move // SPACE jump // E interact/stand // SHIFT sprint\");\n",
    "    SDL_Log(\"[HAKUI] controls // WASD move // SPACE jump // E interact/stand // SHIFT sprint // F6 body\");\n",
    "document F6 body switch",
)

replace_once(
    app,
    "        {\"avatar.rig.bones\", static_cast<std::int64_t>(avatarSkeleton_.boneCount())},\n"
    "        {\"player.locomotion\", std::string(\"on_foot\")}\n",
    "        {\"avatar.rig.bones\", static_cast<std::int64_t>(avatarSkeleton_.boneCount())},\n"
    "        {\"avatar.body_profile\", std::string(bodyProfile_.active().name)},\n"
    "        {\"player.locomotion\", std::string(\"on_foot\")}\n",
    "publish initial body profile state",
)

replace_once(
    app,
    "    inputBridge_.observeEvent(event);\n",
    "    if (!paused_ && event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat &&\n"
    "        event.key.scancode == SDL_SCANCODE_F6) {\n"
    "        bodyProfile_.toggle();\n"
    "        const std::string profileName{bodyProfile_.active().name};\n"
    "        showInputStatus(\"BODY PROFILE // \" + profileName);\n"
    "        SDL_Log(\"[HAKUI] body profile // %s\", profileName.c_str());\n"
    "        recordObserverEvent(\"avatar.body_profile\", profileName);\n\n"
    "        spiral::Signal signal;\n"
    "        signal.kind = spiral::SignalKind::State;\n"
    "        signal.source = \"hakui.avatar\";\n"
    "        signal.destination = \"spiral.core\";\n"
    "        signal.topic = \"avatar.body_profile.changed\";\n"
    "        signal.payload = profileName;\n"
    "        signal.statePatch = {\n"
    "            {\"avatar.body_profile\", profileName}\n"
    "        };\n"
    "        spiral_.dispatch(std::move(signal));\n"
    "        return SDL_APP_CONTINUE;\n"
    "    }\n\n"
    "    inputBridge_.observeEvent(event);\n",
    "add F6 runtime body-profile switch",
)

replace_once(
    app,
    "    HakuiSceneState scene;\n"
    "    scene.paused = paused_;\n",
    "    HakuiSceneState scene;\n"
    "    scene.playerBodyProfile = bodyProfile_.activeId();\n"
    "    scene.paused = paused_;\n",
    "propagate runtime profile to renderer",
)

marker.write_text(
    "# Runtime Body Profiles v0.1\n\n"
    "HAKUI now owns one runtime body-profile controller. Male and female are "
    "presentation profiles on the same skeleton, movement, combat, ride, seat, "
    "interaction and world simulation.\n\n"
    "- default: `male`\n"
    "- startup override: `HAKUI_BODY_PROFILE=male|female`\n"
    "- runtime toggle: `F6`\n"
    "- canonical state key: `avatar.body_profile`\n\n"
    "Mannequin Lab remains an isolated rig-science surface and resolves its "
    "male/female shell through the same immutable `BodyProfile` data.\n",
    encoding="utf-8",
)

print("[HAKUI BODY v0.1] runtime body-profile integration complete")
