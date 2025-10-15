#include "GameManager.hpp"

#include "game/Player.hpp"
#include "game/Sound.hpp"

#include "SDL3/SDL_keyboard.h"

#include "Syngine/Syngine.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/BatchRenderer.hpp"
#include "Syngine/modules/Framebuffer.hpp"
#include "Syngine/modules/Presets.hpp"
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
#include "LinearMath/btVector3.h"

#include "glm/fwd.hpp"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include "input/InputProcessor.hpp"

#include <string>
#include <unordered_map>
#include <vector>

static std::unordered_map<std::string, std::vector<Sound::WavData>> wavSteps;

static BT_World *world;
static BT_EntityTriangleMesh *sceneEntity;
static btRigidBody *playerBody;

static Model *sceneColliderModel;
static Model *sceneModel;

static ModelInstance *sceneColliderModelInst;
static ModelInstance *sceneModelInst;

static ModelBatchRenderer *modelBatch;

static GameManager::GameState state = GameManager::State_Menu;

static Mesh2D *crosshair;

static glm::vec3 fallbackColor = {0.5f, 0.4f, 0.3f};
static float gamma_value = 2.4f;

static Scene *scene;
static Framebuffer *mainFB;
static Camera camera({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});

static Shader batchShader, batch2DShader, screenShader;

GameManager::ShaderRenderable_Task::ShaderRenderable_Task(
    std::function<void(Shader& shader, Screenbuffer screen)> func
) : func(func) {}

void GameManager::ShaderRenderable_Task::render(Shader& shader, Screenbuffer screen) {
    this->func(shader, screen);
}

GameManager::GameState getState() {
    return state;
}

void GameManager::setState(GameState _state) {
    switch (_state) {
        case State_Menu:

        break;
        case State_OutView:
        
        break;
        case State_Interior_0:
        break;
        case State_Interior_1:
        break;
    }
    state = _state;
}

std::vector<Sound::WavData> &GameManager::getWavSteps(const std::string& mat_name) {
    std::string key = "";

    static std::vector<std::string> knownNames = {"Road", "Sand", "Rock", "Metal"};
    for (auto knownName : knownNames) {
        if (mat_name.rfind(knownName, 0) == 0) {
            key = knownName;
            break;
        }
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
    wavSteps["Road"] = wavSteps["Rock"] = road_steps;

    std::vector<Sound::WavData> sand_steps;
    sand_steps.push_back(Sound::loadWav("sounds/166511__yoyodaman234__dirtgravel-footstep-1.wav"));
    sand_steps.push_back(Sound::loadWav("sounds/223154__yoyodaman234__dirtgravel-footstep-3.wav"));
    wavSteps["Sand"] = sand_steps;

    std::vector<Sound::WavData> metal_steps;
    metal_steps.push_back(Sound::loadWav("sounds/421134__giocosound__footstep_metal_1.wav"));
    metal_steps.push_back(Sound::loadWav("sounds/421137__giocosound__footstep_metal_4.wav"));
    metal_steps.push_back(Sound::loadWav("sounds/421136__giocosound__footstep_metal_5.wav"));
    metal_steps.push_back(Sound::loadWav("sounds/421133__giocosound__footstep_metal_2.wav"));
    wavSteps["Metal"] = metal_steps;

    batchShader.read("shaders/ps1batchVertex.glsl", "shaders/ps1batchFrag.glsl");
    batchShader.init();

    batch2DShader.read("shaders/batch2DVertex.glsl", "shaders/batch2DFrag.glsl");
    batch2DShader.init();
 
    screenShader.read("shaders/screenVertex.glsl", "shaders/screenFrag.glsl");
    screenShader.init();

    Vertex2D min = {{0, 0}, {0.0f, 0.0f}};
    Vertex2D max = {{0.005f, 0.005f}, {1.0f, 1.0f}};
    crosshair = Presets2D::newMeshQuad(min, max, 0);

    GLubyte color[] = {255, 255, 255, 255};
    crosshair->setFallbackColor(color);
    crosshair->init();

    sceneModel = new Model();
    sceneModel->readAssimp({"models/outdoor lab/test/scenetest.gltf"});
    sceneModel->uploadVertices(syng::CacheApproach::Interleaved);
    sceneModelInst = new ModelInstance(sceneModel);

    sceneColliderModel = new Model();
    sceneColliderModel->readAssimp({"models/outdoor lab/test_collider/sceneCollider.gltf"});
    sceneColliderModel->groupMeshes();
    sceneColliderModelInst = new ModelInstance(sceneColliderModel);

    scene = new Scene(&camera, batchShader, screenShader, Scene_T::of({1024, 768}));
    scene->getBatchShader().use();
    scene->getBatchShader().setVec2f("screenSize", 480, 360);

    modelBatch = new ModelBatchRenderer(scene);
    modelBatch->add("sceneModel", sceneModelInst);
    scene->getBatchRenderTable()->add("modelBatch", modelBatch);

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
    mainFB->getRenderTable()->add("crosshair", new ShaderRenderable_Task([](Shader& shader, Screenbuffer screen){
        crosshair->render(batch2DShader, *mainFB, glm::mat4(1.0f));
    }));
    mainFB->create(480, 360, true);
    
    sceneEntity = new BT_EntityTriangleMesh(sceneColliderModelInst);
    sceneEntity->load();

    world = new BT_World(0, "World");

    Coordination playerPos = {{0.0f, 2.0f, 8.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}};
    camera.setDirection(playerPos.getDirection());
    world->getDynamics()->addRigidBody(playerBody = GamePlay::Player::createEntity(playerPos, 40.0f, 0.35f, 1.8f));
    world->getDynamics()->addRigidBody(sceneEntity->getBody());
    world->paused = false;

    window->getWindowRenderTable()->add("main", mainFB);
    window->getWindowRenderTable()->add("world", world);
    window->addEventHandler(new GameInput::MouseLookHandler(camera));
    window->addEventHandler(scene);

    SDL_GL_SetSwapInterval(0);
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
