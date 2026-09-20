#pragma once

#include <array>
#include <string_view>

#include <SDL3/SDL.h>

#include "body/BodyPoseSolver.hpp"
#include "player/PlayerMovementController.hpp"
#include "player/PlayerState.hpp"
#include "player/RideableMovementController.hpp"
#include "render/DebugWorldRenderer.hpp"
#include "world/WorldGeometry.hpp"

class MannequinLabApp {
public:
    bool boot();
    SDL_AppResult handleEvent(const SDL_Event& event);
    SDL_AppResult tick();
    void shutdown();

private:
    enum class PosePreset : unsigned char {
        Neutral = 0,
        TPose,
        APose,
        Crouch,
        OlliePop,
        Locomotion = 255
    };

    bool initPlatform();
    bool initGPU();
    bool render();
    void applyPreset(PosePreset preset);
    void updateWindowTitle();
    void adjustKnees(float delta);
    void adjustPelvisYaw(float delta);
    void adjustTorsoYaw(float delta);
    void adjustTorsoLean(float delta);
    std::string_view poseLabel() const noexcept;
    hakui::body::BodyPosePreset solverPreset() const noexcept;

private:
    SDL_Window* window_ = nullptr;
    SDL_GPUDevice* gpu_ = nullptr;
    DebugWorldRenderer renderer_{};
    PlayerState mannequin_{};
    hakui::PlayerMovementController movement_{};
    hakui::body::BodyPoseSolver poseSolver_{};
    hakui::body::BodyPoseState solvedPose_{};
    hakui::RideBodyMechanicsState pose_{};
    PosePreset preset_ = PosePreset::Locomotion;
    bool showJoints_ = true;
    bool cameraDragging_ = false;
    bool jumpQueued_ = false;
    Uint64 previousCounter_ = 0;
};
