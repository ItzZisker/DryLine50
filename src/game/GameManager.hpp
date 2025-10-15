#pragma once

#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Mesh.hpp"
#include "Syngine/modules/Shader.hpp"
#include "game/Sound.hpp"

#include "Syngine/Syngine.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/world/World.hpp"

#include "BulletDynamics/Dynamics/btRigidBody.h"

#include <vector>

using namespace syng;

namespace GameManager {
    enum GameState {
        State_Menu,
        State_OutView,
        State_Interior_0,
        State_Interior_1
    };

    class ShaderRenderable_Task : public ShaderRenderable {
    private:
        std::function<void(Shader& shader, Screenbuffer screen)> func;
    public:
        ShaderRenderable_Task(std::function<void(Shader& shader, Screenbuffer screen)> func);
        void render(Shader& shader, Screenbuffer screen) override;
    };

    std::vector<Sound::WavData> &getWavSteps(const std::string& key);
    ModelInstance *getSceneColliderModelInst();
    BT_World *getWorld();
    btRigidBody *getPlayer();

    GameState getState();
    void setState(GameState state);

    void start(GameWindow* window);
    void startImGUI(GameWindow* window);

    void render(GameWindow* window);
    void renderImGUI(GameWindow* window);

    void shutdown(GameWindow* window);
    void shutdownImGUI(GameWindow* window);
}