#include "GameManager.hpp"

#include "LinearMath/btVector3.h"
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

#include "game/Sound.hpp"
#include "glm/fwd.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include "input/InputProcessor.hpp"

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

static std::unordered_map<std::string, std::vector<Sound::WavData>> wavSteps;

static BT_World *world;
static BT_EntityTriangleMesh *sceneEntity;
static btRigidBody *playerBody;

static ModelInstance *sceneColliderModelInst;
static ModelInstance *sceneModelInst;

static glm::vec3 fallbackColor = {0.5f, 0.4f, 0.3f};
static float gamma_value = 2.4f;

static Scene *scene;
static Framebuffer *mainFB;
static Camera camera({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});

static Shader batchShader, screenShader;

std::vector<Sound::WavData> &GameManager::getWavSteps(const std::string& mat_name) {
    std::string key = "";

    if (mat_name.rfind("Road", 0) == 0) {
        key = "Road";
    }
    if (wavSteps.find(key) != wavSteps.end()) {
        return wavSteps[key];
    } else {
        static std::vector<Sound::WavData> empty;
        return empty;
    }
}

ModelInstance *GameManager::getSceneColliderModelInst() {
    return sceneColliderModelInst;
}

BT_World *GameManager::getWorld() {
    return world;
}

btRigidBody *GameManager::getPlayer() {
    return playerBody;
}

void GameManager::start(GameWindow *window) {
    std::vector<Sound::WavData> road_steps;

    road_steps.push_back(Sound::loadWav("sounds/166509__yoyodaman234__concrete-footstep-1.wav"));
    road_steps.push_back(Sound::loadWav("sounds/166508__yoyodaman234__concrete-footstep-2.wav"));
    road_steps.push_back(Sound::loadWav("sounds/166507__yoyodaman234__concrete-footstep-3.wav"));
    road_steps.push_back(Sound::loadWav("sounds/166506__yoyodaman234__concrete-footstep-4.wav"));

    wavSteps["Road"] = road_steps;

    batchShader.read("shaders/ps1batchVertex.glsl", "shaders/ps1batchFrag.glsl");
    batchShader.init();
 
    screenShader.read("shaders/screenVertex.glsl", "shaders/screenFrag.glsl");
    screenShader.init();

    Model *sceneModel = new Model();
    std::cout << "read 0\n";
    sceneModel->readAssimp({"models/outdoor lab/test/scenetest.gltf"});
    std::cout << "read 1\n";
    sceneModel->uploadVertices(syng::CacheApproach::Interleaved);
    sceneModelInst = new ModelInstance(sceneModel);

    Model *sceneColliderModel = new Model();
    sceneColliderModel->readAssimp({"models/outdoor lab/test_collider/sceneCollider.gltf"});
    sceneColliderModel->groupMeshes();
    sceneColliderModelInst = new ModelInstance(sceneColliderModel);

    scene = new Scene(&camera, batchShader, screenShader, Scene_T::of({1024, 768}));
    scene->getBatchShader().use();
    scene->getBatchShader().setVec2f("screenSize", 320, 240);
    scene->getBatchRenderTable()->add("sceneModel", sceneModelInst);
    DirLight noonLight = {
        {0.86f, -1.0f, 0.97f},
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.75f},
        {0.6f, 0.6f, 0.85f}
    };
    noonLight.ambient *= 0.3f;
    noonLight.diffuse *= 0.3f;
    noonLight.specular *= 0.3f;
    scene->setDirectionalLight(noonLight);
    scene->setPointLights({{{0.0f, 0.0f, 0.0f}}});
    scene->reloadShaders();

    mainFB = new Framebuffer(scene);
    mainFB->setFallbackColor({0.5f, 0.5f, 0.2f});
    mainFB->setTCBFiltering(GL_NEAREST);
    mainFB->getRenderTable()->add("scene", scene);
    mainFB->create(480, 240, true);
    
    sceneEntity = new BT_EntityTriangleMesh(sceneColliderModelInst);
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
    glm::vec3 camPos = {position.x(), position.y() + 1.3f, position.z()};
    camera.setPosition(camPos);
    scene->setGamma(gamma_value);
    scene->getBatchShader().use();
    scene->getBatchShader().setVec3f("cameraPos", camPos[0], camPos[1], camPos[2]);
    mainFB->setFallbackColor(fallbackColor);

    GameInput::handleKeysToggleMouseLook(SDL_GetKeyboardState(NULL), window->getSDLWindowPtr());
    GameInput::handleKeysMovement(SDL_GetKeyboardState(NULL), playerBody, world, 1.8f, window->getLastFrameTime());

    btVector3 pos = playerBody->getWorldTransform().getOrigin();
    Sound::setListenerPosition({pos.x(), pos.y(), pos.z()});

    if (playerBody->getLinearVelocity().length2() <= 0.1f) {
        Sound::Footsteps::end();
    }
}

void GameManager::shutdown(GameWindow *window) {
    window->getWindowRenderTable()->remove("main");
    scene->getBatchRenderTable()->remove("sceneModel");
    delete scene;
    batchShader.disposeProgram();
    screenShader.disposeProgram();
    world->getDynamics()->removeRigidBody(sceneEntity->getBody());
    world->getDynamics()->removeRigidBody(playerBody);
    world->paused = true;
    window->getWindowRenderTable()->remove("world");
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
