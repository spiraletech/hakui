#pragma once

#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

#include <SDL3/SDL.h>

#include "audio/HakuiAudio.hpp"
#include "avatar/BodyProfileController.hpp"
#include "avatar/HakuiSkeleton.hpp"
#include "combat/CombatSimulation.hpp"
#include "action/HakuiNpcActionExecutor.hpp"
#include "core/GameRuntime.hpp"
#include "games/GameTerminal.hpp"
#include "interaction/InteractionService.hpp"
#include "input/SdlInputBridge.hpp"
#include "input/RideControlInterpreter.hpp"
#include "intent/HakuiIntentProposal.hpp"
#include "observer/ExpertObserver.hpp"
#include "render/DebugWorldRenderer.hpp"
#include "social/ChatSystem.hpp"
#include "spiral/SpiralKernel.hpp"
#include "spiral/hakui/HakuiAdapter.hpp"
#include "spiral/hakui/SpiralCortexClient.hpp"
#include "spiral/hakui/SpiralPresence.hpp"
#include "systems/LocomotionRouter.hpp"

class HakuiApp {
public:
    bool boot();
    SDL_AppResult handleEvent(const SDL_Event& event);
    SDL_AppResult tick();
    void shutdown();

private:
    struct CortexMailbox {
        std::mutex mutex;
        std::optional<hakui::SpiralCortexReply> reply;
    };

    bool initPlatform();
    bool initGPU();
    void initSpiralCore();
    void switchLocomotion(LocomotionMode mode, std::string_view label);
    void requestRideLocomotion(LocomotionMode mode, std::string_view label);
    void showInputStatus(std::string message, float seconds = 2.8f);
    void interactWithTerminal(hakui::InteractionVerb verb);
    void handleCasinoContextAction();
    void leaveCurrentInteraction();
    void handlePrimaryInteraction();
    void toggleCombat();
    void updateCombat(float dt, const hakui::input::DisciplineIntent& intent);
    bool setCameraCapture(bool enabled);
    void setPaused(bool paused);
    void beginChatInput();
    void commitChatInput();
    void cancelChatInput();
    void refreshCortexBinding();
    void beginCortexRequest(std::string prompt);
    void pollCortex();
    void openGamepad(SDL_JoystickID instanceId);
    void updateHud();
    void update(float dt);
    bool render();
    void recordObserverEvent(std::string_view category, std::string_view message);
    hakui::observer::CaptureContext buildObserverContext() const;
    void captureExpertSnapshot();

    SDL_Window* window_ = nullptr;
    SDL_GPUDevice* gpu_ = nullptr;
    SDL_Gamepad* gamepad_ = nullptr;
    Uint64 previousCounter_ = 0;
    float titleTimer_ = 0.0f;
    float footstepDistance_ = 0.0f;
    bool cameraDragging_ = false;
    int cameraCaptureWarmupFrames_ = 0;
    bool paused_ = false;
    bool quitRequested_ = false;
    bool developerRideFallback_ = false;
    bool alternateFootstep_ = false;
    float inputStatusTimer_ = 0.0f;
    std::string inputStatus_;
    float opponentDecisionTimer_ = 1.15f;
    float playerHitPulse_ = 0.0f;
    float opponentHitPulse_ = 0.0f;
    bool opponentCrossNext_ = false;
    bool expertCaptureRequested_ = false;
    float socialPreviewCaptureDelay_ = -1.0f;
    std::filesystem::path lastObserverBundle_;

    // Spiral is the client's orchestration spine. It observes/receives typed
    // interaction lifecycle signals but does not own deterministic game state.
    spiral::SpiralKernel spiral_;
    spiral::RouterBus::ListenerId spiralListener_ = 0;

    // L5 authority split. GameRuntime owns exactly one world authority, one
    // player authority and one interaction-membership authority. These
    // compatibility references preserve the existing native client while
    // keeping all deterministic state rooted below GameRuntime.
    hakui::GameRuntime runtime_{};
    hakui::HakuiWorldState& world_ = runtime_.world();
    hakui::BlackRoom& blackRoom_ = runtime_.blackRoom();
    PlayerState& player_ = runtime_.player();
    hakui::PlayerMovementController& movement_ = runtime_.movement();
    hakui::RideableMovementController& rideable_ = runtime_.rideable();

    // L7 read-only perception boundary. The native client owns one adapter
    // bound to the authoritative runtime. It can capture/inspect HakuiSnapshot
    // truth but has no mutation, interaction-execution or Spiral-write path.
    hakui::HakuiAdapter hakuiAdapter_{runtime_};

    // L9 cortex transport is process-isolated and loopback-only. The client
    // sends frozen L6 snapshots to the real Spiral Ether AI HostKind::Hakui
    // runtime and receives text only. A detached worker owns no HAKUI refs;
    // replies return through a shared mailbox on the main thread.
    hakui::SpiralCortexClient cortexClient_{};
    hakui::SpiralCortexStatus cortexStatus_{};
    hakui::HakuiNpcActionExecutor npcActionExecutor_{};
    std::uint64_t nextProposalId_ = 1;
    std::shared_ptr<CortexMailbox> cortexMailbox_ =
        std::make_shared<CortexMailbox>();
    float cortexProbeTimer_ = 0.0f;

    // L8 visible embodiment of the read-only HAKUI link. L9 feeds it only a
    // cortex status value; presence itself still owns no model or action path.
    hakui::SpiralPresence spiralPresence_{hakuiAdapter_};

    // Execution + Spiral telemetry are separate from target membership. The
    // service is wired to the registry owned by GameRuntime.
    hakui::InteractionService interactions_{
        runtime_.interactionRegistry(),
        spiral_.router()
    };
    std::shared_ptr<hakui::games::GameTerminal> terminal_;

    // LocomotionRouter still emits SDL diagnostics, so it remains in the
    // platform shell until its logging contract is made dependency-free.
    LocomotionRouter locomotion_{player_};

    HakuiSkeleton avatarSkeleton_;
    hakui::avatar::BodyProfileController bodyProfile_{};
    HakuiAudio audio_;
    hakui::input::SdlInputBridge inputBridge_;
    hakui::input::InputFrame inputFrame_{};
    hakui::input::RideControlInterpreter rideControls_{};
    hakui::input::RideControlFrame rideControlFrame_{};
    DebugWorldRenderer debugRenderer_;
    hakui::combat::CombatSimulation combat_;
    hakui::social::ChatSystem chat_;
    hakui::observer::RuntimeEventJournal observerJournal_{96};
};
