#pragma once

#include "game/Sound.hpp"

#include "Syngine/Syngine.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/world/World.hpp"

#include "BulletDynamics/Dynamics/btRigidBody.h"

#include <vector>

using namespace syng;

namespace GameManager {
    std::vector<Sound::WavData> &getWavSteps(const std::string& key);
    ModelInstance *getSceneColliderModelInst();
    BT_World *getWorld();
    btRigidBody *getPlayer();

    void start(GameWindow* window);
    void startImGUI(GameWindow* window);

    void render(GameWindow* window);
    void renderImGUI(GameWindow* window);

    void shutdown(GameWindow* window);
    void shutdownImGUI(GameWindow* window);
}