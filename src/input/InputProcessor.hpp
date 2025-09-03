#pragma once

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "Syngine/modules/Camera.hpp"
#include "Syngine/modules/EventHandler.hpp"

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_video.h"
#include "Syngine/world/World.hpp"

namespace GameInput
{
    float getMouseLookYaw();
    float getMouseLookPitch();

    double getMouseLastX();
    double getMouseLastY();
    float getMouseSensitivity();

    bool isMouseCaptured();

    void setMouseLookYaw(float yaw);
    void setMouseLookPitch(float pitch);

    void setMouseSensitivity(float sens);
    void setMouseCaptured(bool captured);

    void onMouseLook(const SDL_Event& mouseEvent, syng::Camera &camera);

    void handleKeysToggleMouseLook(const bool* SDL_keyStates, SDL_Window *SDL_window);
    void handleKeysMovement(const bool* SDL_keyStates, syng::Camera &camera, syng::BT_World *world, btRigidBody *player, float capsuleHeight, double lastFrameTime);

    class MouseLookHandler : public syng::SDL_EventHandler {
    private:
        syng::Camera& camera;
    public:
        MouseLookHandler(syng::Camera& camera) : camera(camera) {}

        void onEvent(const SDL_Event& event) override {
            GameInput::onMouseLook(event, camera);
        }
    };
}