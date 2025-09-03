#pragma once

#include "BulletDynamics/Dynamics/btRigidBody.h"
#include "Syngine/Syngine.hpp"
#include "Syngine/world/World.hpp"

using namespace syng;

namespace GameManager
{
    BT_World *getWorld();
    btRigidBody *getPlayer();

    void start(GameWindow* window);
    void startImGUI(GameWindow* window);

    void render(GameWindow* window);
    void renderImGUI(GameWindow* window);

    void shutdown(GameWindow* window);
    void shutdownImGUI(GameWindow* window);
}