#pragma once

#include "AL/al.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <vector>

namespace Sound {
    struct WavData {
        std::vector<char> data;
        ALenum format;
        ALsizei size;
        ALsizei freq;
    };

    bool createContext(const ALchar *devicename = nullptr);
    void shutdown();

    void setListenerPosition(const glm::vec3& pos);
    void setListenerVelocity(const glm::vec3& vel);
    void setListenerOrientation(const glm::vec3& forward, const glm::vec3& up);

    WavData loadWav(const std::filesystem::path& path);
    void playWav(const WavData& wav, const glm::vec3& pos, const glm::vec3& vel = glm::vec3(0.0f));

    namespace Footsteps {
        void begin(const std::vector<WavData>& steps, const glm::vec3& pos, const glm::vec3& vel);
        void end();
    }
}

