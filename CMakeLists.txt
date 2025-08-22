cmake_minimum_required(VERSION 3.10)
project(DryLine50 VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# === Include headers ===
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/libs/zstd-1.5.7/include
    ${CMAKE_SOURCE_DIR}/libs/syngine-0.1.0/include
    ${CMAKE_SOURCE_DIR}/libs/sdl-3.2.18/include
    ${CMAKE_SOURCE_DIR}/libs/glad-3.3/include
    ${CMAKE_SOURCE_DIR}/libs/stb-image
    ${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/include
    ${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/build/include
    ${CMAKE_SOURCE_DIR}/libs/bullet3-3.25/src
    ${CMAKE_SOURCE_DIR}/libs/glm
    ${CMAKE_SOURCE_DIR}/libs/imgui-1.91.9b
    ${CMAKE_SOURCE_DIR}/libs/imgui-1.91.9b/backends
)

# === Link directories ===
link_directories(
    ${CMAKE_SOURCE_DIR}/libs/syngine-0.1.0/bin
    ${CMAKE_SOURCE_DIR}/libs/sdl-3.2.18/lib
    ${CMAKE_SOURCE_DIR}/libs/assimp-6.0.2/build/lib
)

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

# === Link libraries to executable ===
target_link_libraries(Game PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/zstd-1.5.7/static/libzstd_static.lib
    Syng
    ${common_libs}
)
