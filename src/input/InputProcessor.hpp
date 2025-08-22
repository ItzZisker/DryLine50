#pragma once

#include "SDL3/SDL_events.h"
#include "SDL3/SDL_video.h"

#include "Syngine/modules/Camera.hpp"

#include "glm/glm.hpp"

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
    void updateKeyStates(double lastFrameTime, const bool* SDL_keyStates, SDL_Window* SDL_window, syng::Camera& camera);
}