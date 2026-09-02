from pathlib import Path

PATH = Path("src/core/HakuiApp.cpp")
text = PATH.read_text(encoding="utf-8")


def replace_once(old: str, new: str, label: str) -> None:
    global text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"{label}: expected exactly one match, found {count}")
    text = text.replace(old, new, 1)


replace_once(
    '#include <string>\n#include <utility>\n',
    '#include <string>\n#include <thread>\n#include <utility>\n',
    "thread include",
)

replace_once(
    '    initSpiralCore();\n    terminal_ = std::make_shared<hakui::games::GameTerminal>(\n',
    '    initSpiralCore();\n    refreshCortexBinding();\n    terminal_ = std::make_shared<hakui::games::GameTerminal>(\n',
    "boot cortex probe",
)

replace_once(
    '''    const hakui::SpiralPresenceView bootPresence = spiralPresence_.view();
    SDL_Log(
        "[HAKUI] SPIRAL PRESENCE // %s // CORTEX UNBOUND",
        bootPresence.linked ? "HAKUI LINK READ ONLY" : "LINK OFFLINE"
    );
''',
    '''    const hakui::SpiralPresenceView bootPresence = spiralPresence_.view(
        hakui::SpiralPresence::defaultNearbyRadius,
        cortexStatus_
    );
    SDL_Log(
        "[HAKUI] SPIRAL PRESENCE // %s // %s",
        bootPresence.linked ? "HAKUI LINK READ ONLY" : "LINK OFFLINE",
        bootPresence.cortexLine.c_str()
    );
''',
    "boot presence status",
)

replace_once(
    '    SDL_Log("[HAKUI] social language // ENTER chat // ENTER send // ESC cancel");\n',
    '    SDL_Log("[HAKUI] social language // ENTER chat // near Spiral node ENTER talks to cortex // ESC cancel");\n',
    "boot controls",
)

old_commit = '''void HakuiApp::commitChatInput()
{
    if (!chat_.inputActive()) {
        return;
    }
    const hakui::social::ChatMessage* message = chat_.commitLocal(
        1,
        static_cast<double>(world_.elapsedSeconds)
    );
    if (window_) {
        SDL_StopTextInput(window_);
    }
    if (message) {
        recordObserverEvent("social.local", "avatar chat message committed");
        SDL_Log(
            "[SOCIAL] LOCAL // intent %.*s // %s",
            static_cast<int>(hakui::social::speechIntentLabel(
                message->speechIntent
            ).size()),
            hakui::social::speechIntentLabel(message->speechIntent).data(),
            message->text.c_str()
        );
    } else {
        recordObserverEvent("social.input", "empty chat input dismissed");
    }
}
'''

new_commit = '''void HakuiApp::commitChatInput()
{
    if (!chat_.inputActive()) {
        return;
    }

    const std::string prompt = chat_.inputBuffer();
    const hakui::SpiralPresenceView presence = spiralPresence_.view(
        hakui::SpiralPresence::defaultNearbyRadius,
        cortexStatus_
    );
    const bool routeToCortex = presence.playerInRange;

    const hakui::social::ChatMessage* message = chat_.commitLocal(
        1,
        static_cast<double>(world_.elapsedSeconds)
    );
    if (window_) {
        SDL_StopTextInput(window_);
    }
    if (message) {
        recordObserverEvent(
            routeToCortex ? "spiral.cortex.user" : "social.local",
            routeToCortex
                ? "human message routed to Spiral cortex"
                : "avatar chat message committed"
        );
        SDL_Log(
            routeToCortex
                ? "[SPIRAL CORTEX] USER // %s"
                : "[SOCIAL] LOCAL // %s",
            message->text.c_str()
        );
        if (routeToCortex) {
            beginCortexRequest(prompt);
        }
    } else {
        recordObserverEvent("social.input", "empty chat input dismissed");
    }
}
'''
replace_once(old_commit, new_commit, "commitChatInput")

insert_after_cancel = '''void HakuiApp::cancelChatInput()
{
    if (!chat_.inputActive()) {
        return;
    }
    chat_.cancelInput();
    if (window_) {
        SDL_StopTextInput(window_);
    }
    recordObserverEvent("social.input", "chat input cancelled");
    SDL_Log("[SOCIAL] CHAT INPUT // CANCELLED");
}
'''

cortex_methods = insert_after_cancel + '''

void HakuiApp::refreshCortexBinding()
{
    if (cortexStatus_.busy) {
        return;
    }

    const bool wasBound = cortexStatus_.bound;
    const hakui::SpiralCortexReply probe = cortexClient_.probe();
    cortexStatus_.bound = probe.connected && probe.ok;
    cortexStatus_.localModelLoaded = probe.localModelLoaded;
    cortexStatus_.model = probe.model;
    cortexStatus_.detail = probe.ok ? probe.text : probe.error;
    cortexProbeTimer_ = cortexStatus_.bound ? 5.0f : 2.0f;

    if (cortexStatus_.bound != wasBound) {
        SDL_Log(
            cortexStatus_.bound
                ? "[SPIRAL CORTEX] BOUND // Spiral Ether AI // model %s"
                : "[SPIRAL CORTEX] OFFLINE // run SpiralHakuiCortex.exe",
            cortexStatus_.bound ? cortexStatus_.model.c_str() : ""
        );
        recordObserverEvent(
            "spiral.cortex.binding",
            cortexStatus_.bound ? "bound" : "offline"
        );
    }
}

void HakuiApp::beginCortexRequest(std::string prompt)
{
    if (prompt.empty()) {
        return;
    }
    if (cortexStatus_.busy) {
        (void)chat_.postSystem(
            "SPIRAL CORTEX BUSY // WAIT FOR CURRENT RESPONSE",
            static_cast<double>(world_.elapsedSeconds),
            hakui::social::MessageSource::SystemAI
        );
        return;
    }

    if (!cortexStatus_.bound) {
        refreshCortexBinding();
    }
    if (!cortexStatus_.bound) {
        (void)chat_.postSystem(
            "SPIRAL CORTEX OFFLINE // RUN SpiralHakuiCortex.exe",
            static_cast<double>(world_.elapsedSeconds),
            hakui::social::MessageSource::SystemAI
        );
        showInputStatus("SPIRAL CORTEX OFFLINE // START BRIDGE", 4.0f);
        return;
    }

    cortexStatus_.busy = true;
    showInputStatus("SPIRAL CORTEX // THINKING", 8.0f);
    const hakui::HakuiSnapshot snapshot = hakuiAdapter_.snapshot();
    const hakui::SpiralCortexClient client = cortexClient_;
    const std::shared_ptr<CortexMailbox> mailbox = cortexMailbox_;
    {
        std::lock_guard<std::mutex> lock(mailbox->mutex);
        mailbox->reply.reset();
    }

    std::thread([
        client,
        snapshot,
        prompt = std::move(prompt),
        mailbox
    ]() mutable {
        hakui::SpiralCortexReply reply = client.ask(snapshot, prompt);
        std::lock_guard<std::mutex> lock(mailbox->mutex);
        mailbox->reply = std::move(reply);
    }).detach();
}

void HakuiApp::pollCortex()
{
    std::optional<hakui::SpiralCortexReply> reply;
    {
        std::lock_guard<std::mutex> lock(cortexMailbox_->mutex);
        if (cortexMailbox_->reply) {
            reply = std::move(cortexMailbox_->reply);
            cortexMailbox_->reply.reset();
        }
    }
    if (!reply) {
        return;
    }

    cortexStatus_.busy = false;
    cortexStatus_.bound = reply->connected;
    cortexStatus_.localModelLoaded = reply->localModelLoaded;
    cortexStatus_.model = reply->model;
    cortexStatus_.detail = reply->ok ? reply->text : reply->error;
    cortexProbeTimer_ = cortexStatus_.bound ? 5.0f : 1.0f;

    if (reply->ok) {
        (void)chat_.postSystem(
            reply->text,
            static_cast<double>(world_.elapsedSeconds),
            hakui::social::MessageSource::SystemAI
        );
        showInputStatus("SPIRAL CORTEX // RESPONSE RECEIVED", 3.0f);
        SDL_Log("[SPIRAL CORTEX] RESPONSE // %s", reply->text.c_str());
        recordObserverEvent("spiral.cortex.reply", "response received");
    } else {
        const std::string notice = "SPIRAL CORTEX ERROR // " + reply->error;
        (void)chat_.postSystem(
            notice,
            static_cast<double>(world_.elapsedSeconds),
            hakui::social::MessageSource::SystemAI
        );
        showInputStatus("SPIRAL CORTEX // ERROR", 4.0f);
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "[SPIRAL CORTEX] %s",
            reply->error.c_str()
        );
        recordObserverEvent("spiral.cortex.error", reply->error);
    }
}
'''
replace_once(insert_after_cancel, cortex_methods, "cortex methods")

replace_once(
    '    const hakui::SpiralPresenceView spiralPresenceView = spiralPresence_.view();\n',
    '''    const hakui::SpiralPresenceView spiralPresenceView = spiralPresence_.view(
        hakui::SpiralPresence::defaultNearbyRadius,
        cortexStatus_
    );
''',
    "HUD presence view",
)

old_chat_title = '''    if (chat_.inputActive()) {
        SDL_snprintf(
            title,
            sizeof(title),
            "HAKUI v1.01 // CHAT INPUT // %s_ // ENTER SEND // ESC CANCEL // %zu/%zu // INPUT ChatInput",
            chat_.inputBuffer().c_str(),
            chat_.inputCodepoints(),
            chat_.tuning().maximumMessageCodepoints
        );
'''
new_chat_title = '''    if (chat_.inputActive()) {
        SDL_snprintf(
            title,
            sizeof(title),
            spiralPresenceView.playerInRange
                ? "HAKUI v1.01 // SPIRAL INPUT // %s_ // ENTER SEND TO CORTEX // ESC CANCEL // %zu/%zu // INPUT ChatInput"
                : "HAKUI v1.01 // CHAT INPUT // %s_ // ENTER SEND // ESC CANCEL // %zu/%zu // INPUT ChatInput",
            chat_.inputBuffer().c_str(),
            chat_.inputCodepoints(),
            chat_.tuning().maximumMessageCodepoints
        );
'''
replace_once(old_chat_title, new_chat_title, "chat title")

old_presence_hud = '''            SDL_snprintf(
                title,
                sizeof(title),
                "HAKUI v1.01 // SPIRAL PRESENCE // HAKUI LINK %s // CORTEX UNBOUND // WORLD %s // NEARBY %zu // INPUT %.*s",
                spiralPresenceView.linked ? "READ ONLY" : "OFFLINE",
                spiralPresenceView.worldLine.c_str(),
                spiralPresenceView.nearbyObjectCount,
                static_cast<int>(device.size()), device.data()
            );
'''
new_presence_hud = '''            SDL_snprintf(
                title,
                sizeof(title),
                "HAKUI v1.01 // SPIRAL PRESENCE // HAKUI LINK %s // %s // ENTER TALK // WORLD %s // NEARBY %zu // INPUT %.*s",
                spiralPresenceView.linked ? "READ ONLY" : "OFFLINE",
                spiralPresenceView.cortexLine.c_str(),
                spiralPresenceView.worldLine.c_str(),
                spiralPresenceView.nearbyObjectCount,
                static_cast<int>(device.size()), device.data()
            );
'''
replace_once(old_presence_hud, new_presence_hud, "presence HUD")

replace_once(
    '''    inputFrame_ = inputBridge_.sample(gamepad_, dt, cameraDragging_);
    inputStatusTimer_ = std::max(0.0f, inputStatusTimer_ - dt);
    chat_.update(dt);
''',
    '''    inputFrame_ = inputBridge_.sample(gamepad_, dt, cameraDragging_);
    inputStatusTimer_ = std::max(0.0f, inputStatusTimer_ - dt);
    cortexProbeTimer_ = std::max(0.0f, cortexProbeTimer_ - dt);
    chat_.update(dt);
    pollCortex();
    const hakui::SpiralPresenceView cortexPresence = spiralPresence_.view(
        hakui::SpiralPresence::defaultNearbyRadius,
        cortexStatus_
    );
    if (!cortexStatus_.busy && !cortexStatus_.bound &&
        cortexPresence.playerInRange && cortexProbeTimer_ <= 0.0f) {
        refreshCortexBinding();
    }
''',
    "update cortex poll",
)

replace_once(
    '    const hakui::SpiralPresenceView spiralPresenceView = spiralPresence_.view();\n    HakuiSceneState scene;\n',
    '''    const hakui::SpiralPresenceView spiralPresenceView = spiralPresence_.view(
        hakui::SpiralPresence::defaultNearbyRadius,
        cortexStatus_
    );
    HakuiSceneState scene;
''',
    "render presence view",
)

PATH.write_text(text, encoding="utf-8")
print("L9 native cortex integration applied")
