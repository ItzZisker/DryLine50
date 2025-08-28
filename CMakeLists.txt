cmake_minimum_required(VERSION 3.10)
project(DryLine50 VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED COMPONENTS OpenGL)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# === Include headers ===
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/libs/zstd-1.5.7/include
    ${CMAKE_SOURCE_DIR}/libs/syngine-0.1.0/include
    ${CMAKE_SOURCE_DIR}/libs/glad-3.3/include
    ${CMAKE_SOURCE_DIR}/libs/stb-image
    ${CMAKE_SOURCE_DIR}/libs/tinygltf
    ${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/include
    ${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/build/include
    ${CMAKE_SOURCE_DIR}/libs/bullet3-3.25/src
    ${CMAKE_SOURCE_DIR}/libs/glm
    ${CMAKE_SOURCE_DIR}/libs/imgui-1.91.9b
    ${CMAKE_SOURCE_DIR}/libs/imgui-1.91.9b/backends
)

# === Link directories ===
if (WIN32)
    link_directories(${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/build/lib)
endif()

# == SDL Support for both Windows/Linux Mac Soon ==
set(SDL_ROOT "${CMAKE_SOURCE_DIR}/libs/sdl-3.2.18")
if (WIN32)
    include_directories(${SDL_ROOT}/windows/include)
    link_directories(${SDL_ROOT}/windows/lib)
elseif(UNIX AND NOT APPLE)
    include_directories(${SDL_ROOT}/linux/include)
    link_directories(${SDL_ROOT}/linux/bin)
endif()

# === ImGui sources ===
set(IMGUI_SRC
    ${CMAKE_SOURCE_DIR}/libs/imgui-1.91.9b/imgui.cpp
    ${CMAKE_SOURCE_DIR}/libs/imgui-1.91.9b/imgui_draw.cpp
    ${CMAKE_SOURCE_DIR}/libs/imgui-1.91.9b/imgui_widgets.cpp
    ${CMAKE_SOURCE_DIR}/libs/imgui-1.91.9b/imgui_tables.cpp
    ${CMAKE_SOURCE_DIR}/libs/imgui-1.91.9b/backends/imgui_impl_opengl3.cpp
    ${CMAKE_SOURCE_DIR}/libs/imgui-1.91.9b/backends/imgui_impl_sdl3.cpp
)

# === Common libraries ===
set(common_libs SDL3 assimp)

if (UNIX AND NOT APPLE)
    list(APPEND common_libs OpenGL::OpenGL Xrandr Xi dl)
elseif (WIN32)
    list(APPEND common_libs opengl32)
elseif (APPLE)
    list(APPEND common_libs "-framework OpenGL" "-framework Cocoa" "-framework IOKit" "-framework CoreVideo")
endif()

# === Add executable ===
add_executable(Game
    src/data/DataZSTD.cpp
    src/game/GameManager.cpp
    src/input/InputProcessor.cpp
    src/Main.cpp
    ${IMGUI_SRC}
)

if (WIN32)
    link_directories(${CMAKE_SOURCE_DIR}/libs/syngine-0.1.0/bin)
    target_link_libraries(Game PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/zstd-1.5.7/static/libzstd_static.lib
        ${common_libs}
    )
elseif (UNIX AND NOT APPLE)
    target_link_libraries(Game PRIVATE ${CMAKE_SOURCE_DIR}/libs/syngine-0.1.0/bin/Syng.so)
    target_link_libraries(Game PRIVATE
        ${CMAKE_SOURCE_DIR}/libs/zstd-1.5.7/staticlinux/libzstd.a
        ${common_libs}
    )
endif()
