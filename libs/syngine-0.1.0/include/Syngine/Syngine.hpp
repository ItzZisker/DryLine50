#pragma once
#define USE_ASSIMP
#define USE_BULLET

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "Syngine/engine/TaskQueue.hpp"
#include "Syngine/engine/RenderTable.hpp"
#include "Syngine/modules/EventHandler.hpp"
#include "Syngine/modules/Screenbuffer.hpp"
#include "Syngine/modules/Shader.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <glad/glad.h>

#include <functional>
#include <unordered_map>
#include <vector>

namespace syng
{
struct WindowSize {
    int width, height;
};

class GameWindow : public Screenbuffer {
private:
	SDL_Window *sdlWindowPtr; 
    SDL_GLContext glContext;

    TaskQueue gameLoopQueue;
    RenderTable<WindowRenderable> *windowRenderTable = new RenderTable<WindowRenderable>(); // Objects that being rendered by window

    double lastFrameTime;

    std::unordered_map<int, int> window_attributes;
    std::unordered_map<std::string, LazyShader> presetShaders;

    std::vector<std::function<void(GameWindow *)>> initTasks, renderTasks, cleanupTasks;
    
    std::vector<SDL_EventHandler*> eventHandlers;
    std::vector<SDL_Event> lastFrameEvents;

    std::string title;

    int sdlWindowStatus, glfwWindowStatus, gladLoadStatus;
    bool initialized, disposed;
public:
    GameWindow(std::string title, WindowSize initialSize);

    bool isInitialized();
    void attrib(SDL_GLAttr attr, int value);
    int initLoop();

    void addEventHandler(SDL_EventHandler *handler);
    void pullEventHandler(SDL_EventHandler *handler);
    void forEachFrameEvents(std::function<void(const SDL_Event event)> func);

    void addInitTask(std::function<void(GameWindow *)> task);
    void addRenderTask(std::function<void(GameWindow *)> task);
    void addCleanupTask(std::function<void(GameWindow *)> task);

    void closeWindow();

    int getGLADLoadStatus();
	int getSDLWindowStatus();
    int getGLFWWindowStatus();

    LazyShader getPresetShader(std::string path);
    double getLastFrameTime();
    std::vector<SDL_Event> getLastFrameEvents();
    TaskQueue& getGameLoopQueue();
    RenderTable<WindowRenderable> *getWindowRenderTable();
	SDL_Window *getSDLWindowPtr();
    SDL_GLContext getGLContext();
    WindowSize getSize();
};
}
