#include "GameManager.hpp"

#include "Syngine/modules/Mesh.hpp"
#include "Syngine/modules/MeshInstance.hpp"
#include "Syngine/modules/ModelInstance.hpp"
#include "input/InputProcessor.hpp"

#include "Syngine/Syngine.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/EventHandler.hpp"
#include "Syngine/modules/Framebuffer.hpp"
#include "Syngine/modules/Model.hpp"
#include "Syngine/modules/Scene.hpp"
#include "Syngine/modules/Camera.hpp"
#include "Syngine/modules/GLObjects.hpp"
#include "Syngine/modules/Presets.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Texture.hpp"
#include "Syngine/world/WorldObject.hpp"

#include <iostream>
#include <ostream>
#include <vector>

Model *sceneModel;
ModelInstance *sceneModelInst;

Scene *scene;
Framebuffer *mainFB;
Coordination cubePos;
Camera camera({0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f});

GLVertexElement<glm::vec3> *cube;
Texture2D diffuseColor;

Shader batchShader, screenShader;

template <typename V>
class GLVERenderable : public ShaderRenderable {
private:
    GLVertex<V> &glv;
    Coordination &coords;
public:
    GLVERenderable(GLVertex<V> &glv, Coordination &coords) : glv(glv), coords(coords) {}

    void render(Shader& shader, Screenbuffer screen) override {
        glBindFramebuffer(GL_FRAMEBUFFER, screen.getFBO());
        shader.use();
        shader.setMatrix4("model", coords.getTransform(), 1, false);
        shader.setMatrix4("view", camera.getViewMatrix(), 1, false);
        shader.setMatrix4("projection", scene->getProjection(), 1, false);
        shader.setTexture("texture_diffuse1", GL_TEXTURE_2D, 0, diffuseColor.TCB);
        glv.draw();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

class MouseLookHandler : public SDL_EventHandler {
public:
    void onEvent(const SDL_Event& event) override {
        GameInput::onMouseLook(event, camera);
    }
};

void GameManager::start(GameWindow *window) {
    std::cout << "1\n";
    std::vector<glm::vec3> vertices;
    std::vector<GLuint> indices;
    Presets3D::pushVerticesCube(1.0f, vertices, indices);
    cube = new GLVertexElement<glm::vec3>(vertices, indices);
    cube->reserve();

    std::cout << "2\n";

    batchShader.read("shaders/ps1batchVertex.glsl", "shaders/ps1batchFrag.glsl");
    batchShader.init();
    std::cout << "3\n";

    screenShader.read("shaders/screenVertex.glsl", "shaders/screenFrag.glsl");
    screenShader.init();
    std::cout << "4\n";

    sceneModel = new Model();
    AssimpReader reader = {"models/outdoor lab/test/scenetest.gltf"};
    sceneModel->readAssimp(reader);
    sceneModel->load(syng::CacheApproach::Sequential);

    sceneModelInst = new ModelInstance(sceneModel);
    // MeshInstance *cube034 = sceneModelInst->getMeshInstances()->get("Cube.034");

    // std::cout << "cube034: " << cube034 << std::endl;


    //sceneModelInst->setDiscard("Cube.034", true);
    //sceneModelInst->getMeshInstances()->get("Cube.034")->addPosition({0.0f, 10.0f, 0.0f});

    // sceneModelInst->getMeshInstances()->forEach([&](const std::string& key, MeshInstance *mI){
    //     std::cout << "IA: " << key << std::endl;
        
    //     if (key == "Plane") {
    //         mI->getChildren()->forEach([&](const std::string &key, MeshInstance *mI){
    //             Mesh* mesh = mI->getSelf();
    //             for (auto vertex : mesh->getVertices()) {
    //                 std::cout << "Vertex.pos=" << vertex.position[0] << "," << vertex.position[1] << "," << vertex.position[2] << " Vertex.texCoord=" << vertex.texCoords[0] << "," << vertex.texCoords[1] << std::endl;
    //             }
    //         });
    //     }
    //     //mI->addPosition({0.0f, 1.0f, 0.0f});
    //     auto ptr =mI->getSelf();
    //     if (ptr) {
    //         std::cout << ptr << std::endl;
    //         for (auto t : ptr->textures) {
    //             std::cout << t.texture.path << std::endl;
    //         }
    //     } else {
    //         mI->getChildren()->forEach([&](const std::string& key, MeshInstance *mI){
    //             auto ptr =mI->getSelf();            
    //             std::cout << ptr << std::endl;
    //             for (auto t : ptr->textures) {
    //                 std::cout << t.texture.path << std::endl;
    //             }
    //         });
    //     }
    // });

    scene = new Scene(&camera, batchShader, screenShader, Scene_T::of({1024, 768}));
    scene->getBatchShader().use();
    scene->getBatchShader().setVec2f("screenSize", 320, 240);
    scene->getBatchRenderTable()->add("sceneModel", sceneModelInst);
    DirLight nightlight = {
        {-0.86f, -1.0f, -0.97f},
        {0.5f, 0.5f, 0.5f},
        {0.5f, 0.5f, 0.75f},
        {0.6f, 0.6f, 0.85f}
    };
    scene->setDirectionalLight(nightlight);
    scene->setPointLights({{{0.0f, 0.0f, 0.0f}}});
    scene->reloadShaders();

    mainFB = new Framebuffer(scene);
    mainFB->setTCBFiltering(GL_NEAREST);
    mainFB->getRenderTable()->add("scene", scene);
    mainFB->create(320, 240, true);

    uint8_t pixel[3] = {0, 128, 0};
    diffuseColor.TCB = TCBFromBytes(pixel, 1, 1, 3);

    cubePos.setPosition({2.0f, 0.0f, 0.0f});
    cubePos.setUp({0.0f, 1.0f, 0.0f});

    window->getWindowRenderTable()->add("main", mainFB);
    window->addEventHandler(new MouseLookHandler());
    window->addEventHandler(scene);
}

void GameManager::render(GameWindow *window) {
    GameInput::updateKeyStates(window->getLastFrameTime(), SDL_GetKeyboardState(NULL), window->getSDLWindowPtr(), camera);
}

void GameManager::shutdown(GameWindow *window) {
    delete cube;
    glDeleteTextures(1, &diffuseColor.TCB);
    batchShader.disposeProgram();
}