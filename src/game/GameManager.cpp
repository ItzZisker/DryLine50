#include "GameManager.hpp"

#include "game/Player.hpp"

#include "SDL3/SDL_keyboard.h"

#include "Syngine/Syngine.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/Framebuffer.hpp"
#include "Syngine/modules/Mesh.hpp"
#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Camera.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/world/entity/BT_EntityTriangleMesh.hpp"
#include "Syngine/world/Coordination.hpp"
#include "Syngine/world/World.hpp"

#include "BulletCollision/BroadphaseCollision/btDispatcher.h"
#include "BulletDynamics/Dynamics/btRigidBody.h"

#include "glm/fwd.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include "input/InputProcessor.hpp"

#include <iostream>
#include <ostream>
#include <vector>

BT_World *world;
BT_EntityTriangleMesh *sceneEntity;
btRigidBody *playerBody;

ModelInstance *sceneModelInst;
glm::vec3 fallbackColor = {0.5f, 0.4f, 0.3f};
float gamma_value = 2.4f;

Scene *scene;
Framebuffer *mainFB;
Camera camera({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});

Shader batchShader, screenShader;

BT_World *GameManager::getWorld() {
    return world;
}

btRigidBody *GameManager::getPlayer() {
    return playerBody;
}

void GameManager::start(GameWindow *window) {
    batchShader.read("shaders/ps1batchVertex.glsl", "shaders/ps1batchFrag.glsl");
    batchShader.init();
 
    screenShader.read("shaders/screenVertex.glsl", "shaders/screenFrag.glsl");
    screenShader.init();

    Model *sceneModel = new Model();
    sceneModel->readAssimp({"models/outdoor lab/test/scenetest.gltf"});
    sceneModel->load(syng::CacheApproach::Interleaved);
    sceneModelInst = new ModelInstance(sceneModel);

    scene = new Scene(&camera, batchShader, screenShader, Scene_T::of({1024, 768}));
    scene->getBatchShader().use();
    scene->getBatchShader().setVec2f("screenSize", 320, 240);
    scene->getBatchRenderTable()->add("sceneModel", sceneModelInst);
    DirLight nightlight = {
        {0.86f, -1.0f, 0.97f},
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.75f},
        {0.6f, 0.6f, 0.85f}
    };
    nightlight.ambient *= 0.25f;
    nightlight.diffuse *= 0.25f;
    nightlight.specular *= 0.25f;
    scene->setDirectionalLight(nightlight);
    scene->setPointLights({{{0.0f, 0.0f, 0.0f}}});
    scene->reloadShaders();

    mainFB = new Framebuffer(scene);
    mainFB->setFallbackColor({0.5f, 0.5f, 0.2f});
    mainFB->setTCBFiltering(GL_NEAREST);
    mainFB->getRenderTable()->add("scene", scene);
    mainFB->create(480, 240, true);
    
    sceneEntity = new BT_EntityTriangleMesh(sceneModelInst);
    sceneEntity->load();

    world = new BT_World(0, "World");

    Coordination playerPos = {{0.0f, 2.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
    world->getDynamics()->addRigidBody(playerBody = GamePlay::Player::createEntity(playerPos, 40.0f, 0.35f, 1.8f));
    world->getDynamics()->addRigidBody(sceneEntity->getBody());
    world->paused = false;

    window->getWindowRenderTable()->add("main", mainFB);
    window->getWindowRenderTable()->add("world", world);
    window->addEventHandler(new GameInput::MouseLookHandler(camera));
    window->addEventHandler(scene);
}

void GameManager::render(GameWindow *window) {
    btVector3 position = playerBody->getWorldTransform().getOrigin();
    glm::vec3 camPos = {position.x(), position.y() + 1.62f, position.z()};
    camera.setPosition(camPos);
    scene->setGamma(gamma_value);
    scene->getBatchShader().use();
    scene->getBatchShader().setVec3f("cameraPos", camPos[0], camPos[1], camPos[2]);
    mainFB->setFallbackColor(fallbackColor);

    GameInput::handleKeysToggleMouseLook(SDL_GetKeyboardState(NULL), window->getSDLWindowPtr());
    GameInput::handleKeysMovement(SDL_GetKeyboardState(NULL), camera, world, playerBody, 1.8f, window->getLastFrameTime());
}

void GameManager::shutdown(GameWindow *window) {
    batchShader.disposeProgram();
}

void GameManager::startImGUI(GameWindow *window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForOpenGL(window->getSDLWindowPtr(), window->getGLContext());
    ImGui_ImplOpenGL3_Init("#version 330");
}

void GameManager::renderImGUI(GameWindow *window) {
    window->forEachFrameEvents([](const SDL_Event event){ImGui_ImplSDL3_ProcessEvent(&event);});
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("Debug");
    ImGui::Text("FPS: %.0f", (window->getLastFrameTime() == 0) ? 999.0f : 1.0f / window->getLastFrameTime());
    ImGui::SliderFloat("Gamma", &gamma_value, 0.1f, 5.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("FBX", &fallbackColor[0], 0.1f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("FBY", &fallbackColor[1], 0.1f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::SliderFloat("FBZ", &fallbackColor[2], 0.1f, 1.0f, "%.3f", ImGuiSliderFlags_Logarithmic);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GameManager::shutdownImGUI(GameWindow *window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
