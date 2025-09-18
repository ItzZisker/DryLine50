#include "InputProcessor.hpp"

#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_video.h"

#include "Syngine/modules/Camera.hpp"
#include "Syngine/utils/GameUtils.hpp"
#include "Syngine/world/World.hpp"
#include "game/Player.hpp"

#include <cmath>

bool mouse_first;
bool mouse_captured = false;
float mouse_sens = 0.1f;
float mouse_look_yaw = 90.0f;
float mouse_look_pitch = 0.0f;
double lastX, lastY;

float GameInput::getMouseLookYaw() { return mouse_look_yaw; }
float GameInput::getMouseLookPitch() { return mouse_look_pitch; }

double GameInput::getMouseLastX() { return lastX; }
double GameInput::getMouseLastY() { return  lastY; }

float GameInput::getMouseSensitivity() { return mouse_sens; }

bool GameInput::isMouseCaptured() { return mouse_captured; }

void GameInput::setMouseLookYaw(float yaw) { mouse_look_yaw = yaw; }
void GameInput::setMouseLookPitch(float pitch) { mouse_look_pitch = pitch; }

void GameInput::setMouseSensitivity(float sens) { mouse_sens = sens; }
void GameInput::setMouseCaptured(bool captured) { mouse_captured = captured; }

void GameInput::onMouseLook(const SDL_Event& mouseEvent, syng::Camera& camera) {
    if (!mouse_captured || mouseEvent.type != SDL_EVENT_MOUSE_MOTION) {
		return;
	}
    float x = static_cast<float>(mouseEvent.motion.x);
    float y = static_cast<float>(mouseEvent.motion.y);

    float xrel = static_cast<float>(mouseEvent.motion.xrel);
    float yrel = static_cast<float>(mouseEvent.motion.yrel);

    xrel *= mouse_sens;
    yrel *= mouse_sens;

    mouse_look_yaw += xrel;
    mouse_look_pitch -= yrel;

    mouse_look_yaw = fmod(mouse_look_yaw, 360.0f);
    mouse_look_pitch = glm::clamp(mouse_look_pitch, -89.0f, 89.0f);

    camera.setDirection(syng::GameUtils::directionOf(mouse_look_yaw, mouse_look_pitch));
}

void GameInput::handleKeysMovement(const bool* SDL_keyStates, btRigidBody *player, BT_World *world, float capsuleHeight, double lastFrameTime) {
    glm::vec3 horizontalDir(cos(glm::radians(mouse_look_yaw)), 0, sin(glm::radians(mouse_look_yaw)));
    horizontalDir = glm::normalize(horizontalDir);
    glm::vec3 rightDir = glm::normalize(glm::cross(horizontalDir, glm::vec3(0,1,0)));

    glm::vec3 moveVel(0.0f);
    if (SDL_keyStates[SDL_SCANCODE_W]) moveVel += horizontalDir;
    if (SDL_keyStates[SDL_SCANCODE_S]) moveVel -= horizontalDir;
    if (SDL_keyStates[SDL_SCANCODE_D]) moveVel += rightDir;
    if (SDL_keyStates[SDL_SCANCODE_A]) moveVel -= rightDir;

    if (glm::length(moveVel) > 0.0f) {
        moveVel = glm::normalize(moveVel) * 5.0f;
        GamePlay::Player::castFootstep(player, world, 1.4f);
    }

    btVector3 currentVel = player->getLinearVelocity();
    currentVel.setX(moveVel.x);
    currentVel.setZ(moveVel.z);
    player->setLinearVelocity(currentVel);
}

void GameInput::handleKeysToggleMouseLook(const bool* SDL_keyStates,SDL_Window *SDL_window) {
    static bool escapePressedLastFrame = false;
    if (SDL_keyStates[SDL_SCANCODE_ESCAPE]) {
        if (!escapePressedLastFrame) {
            mouse_captured = !mouse_captured;
            SDL_SetWindowRelativeMouseMode(SDL_window, mouse_captured);
            mouse_first = true;
        }
        escapePressedLastFrame = true;
    } else {
        escapePressedLastFrame = false;
    }
}