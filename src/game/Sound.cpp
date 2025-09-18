#include "Sound.hpp"

#include "Syngine/engine/TaskQueue.hpp"
#include "Syngine/serialization/DataSerializer.hpp"
#include "Syngine/serialization/DataTemplates.hpp"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <ostream>
#include <random>
#include <unordered_map>
#include <vector>
#include <fstream>

static std::unordered_map<std::string, ALuint> gBuffers;

static ALCdevice* gDevice = nullptr;
static ALCcontext* gContext = nullptr;

static std::atomic<bool> gFootstepRunning{false};

bool Sound::createContext(const ALCchar *devicename) {
    if (gDevice) {
        std::cerr << "OpenAL device held onto a device context already!\n";
        return false;
    }

    gDevice = alcOpenDevice(devicename);
    if (!gDevice) {
        std::cerr << "Failed to open OpenAL device: " << (devicename == nullptr ? "Default" : std::string(devicename)) << std::endl;
        return false;
    }

    ALCint attrs[] = { ALC_HRTF_SOFT, ALC_TRUE, 0 };
    gContext = alcCreateContext(gDevice, attrs);
    if (!gContext) {
        alcCloseDevice(gDevice);
        gDevice = nullptr;
        return false;
    }

    alcMakeContextCurrent(gContext);

    ALfloat ori[] = { 0.0f, 0.0f, -1.0f,
                      0.0f, 1.0f,  0.0f };
    alListener3f(AL_POSITION, 0.f, 0.f, 0.f);
    alListener3f(AL_VELOCITY, 0.f, 0.f, 0.f);
    alListenerfv(AL_ORIENTATION, ori);

    return true;
}

void Sound::shutdown() {
    if (gContext) {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(gContext);
        gContext = nullptr;
    }
    if (gDevice) {
        alcCloseDevice(gDevice);
        gDevice = nullptr;
    }
}

ALuint Sound::getALBuffer(const WavData &wav) {
    if (gBuffers.find(wav.name) != gBuffers.end()) {
        return gBuffers[wav.name];
    }
    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, wav.format, wav.data.data(), wav.size, wav.freq);
    gBuffers[wav.name] = buffer;
    return buffer;
}

Sound::WavData Sound::loadWav(syng::DataDeserializer* buffer) {
    syng::DataTemplates::push(buffer, "OpenAL Wav", PCK_HEADER_WAV);

    std::string filename = syng::DataTemplates::read_string(buffer);
    char riff[4]; buffer->read((uint8_t*) riff, 4);
    buffer->skip(4);
    char wave[4]; buffer->read((uint8_t*) wave, 4);

    char fmt[4]; buffer->read((uint8_t*) fmt, 4);

    uint32_t subchunk1Size; buffer->read(reinterpret_cast<uint8_t*>(&subchunk1Size), 4);
    uint16_t audioFormat; buffer->read(reinterpret_cast<uint8_t*>(&audioFormat), 2);
    uint16_t channels; buffer->read(reinterpret_cast<uint8_t*>(&channels), 2);
    uint32_t sampleRate; buffer->read(reinterpret_cast<uint8_t*>(&sampleRate), 4);
    buffer->skip(6);
    uint16_t bitsPerSample; buffer->read(reinterpret_cast<uint8_t*>(&bitsPerSample), 2);

    char dataHeader[4];
    uint32_t dataSize;
    do {
        buffer->read((uint8_t*) dataHeader, 4);
        buffer->read(reinterpret_cast<uint8_t*>(&dataSize), 4);
        if(std::string(dataHeader, 4) != "data")
            buffer->skip(dataSize);
    } while (std::string(dataHeader,4) != "data");

    WavData wav;
    wav.name = filename;
    wav.size = dataSize;
    wav.freq = sampleRate;
    wav.format = (channels == 1 ?
                 (bitsPerSample == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16) :
                 (bitsPerSample == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16));
    wav.data.resize(dataSize);
    buffer->read((uint8_t*) wav.data.data(), dataSize);

    syng::DataTemplates::pop(buffer, "OpenAL Wav", PCK_FOOTER_WAV);
    return wav;
}

Sound::WavData Sound::loadWav(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("Failed to open file: " + path.string());

    char riff[4]; file.read(riff, 4);
    file.ignore(4); // size
    char wave[4]; file.read(wave, 4);

    char fmt[4]; file.read(fmt, 4);
    uint32_t subchunk1Size; file.read(reinterpret_cast<char*>(&subchunk1Size), 4);
    uint16_t audioFormat; file.read(reinterpret_cast<char*>(&audioFormat), 2);
    uint16_t channels; file.read(reinterpret_cast<char*>(&channels), 2);
    uint32_t sampleRate; file.read(reinterpret_cast<char*>(&sampleRate), 4);
    file.ignore(6);
    uint16_t bitsPerSample; file.read(reinterpret_cast<char*>(&bitsPerSample), 2);

    char dataHeader[4];
    uint32_t dataSize;
    do {
        file.read(dataHeader, 4);
        file.read(reinterpret_cast<char*>(&dataSize), 4);
        if(std::string(dataHeader,4) != "data")
            file.ignore(dataSize);
    } while (std::string(dataHeader,4) != "data");

    WavData wav;
    wav.name = path.filename().string();
    wav.size = dataSize;
    wav.freq = sampleRate;
    wav.format = (channels == 1 ?
                 (bitsPerSample == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16) :
                 (bitsPerSample == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16));
    wav.data.resize(dataSize);
    file.read(wav.data.data(), dataSize);
    return wav;
}

void TQ_gClr(ALuint source) {
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);

    if (state != AL_PLAYING) {
        alDeleteSources(1, &source);
    } else {
        syng::TaskQueue::Instance().enqueue([source]() {
            TQ_gClr(source);
        });
    }
}

void Sound::playWav(const WavData& wav, const glm::vec3& pos, const glm::vec3& vel) {
    if (!gDevice || !gContext) return;

    ALuint source;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, getALBuffer(wav));
    alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
    alSource3f(source, AL_VELOCITY, vel.x, vel.y, vel.z);
    alSourcePlay(source);

    syng::TaskQueue::Instance().enqueue([source]() {
        TQ_gClr(source);
    });
}

void Sound::setListenerPosition(const glm::vec3& pos) {
    if (!gDevice || !gContext) return;
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
}

void Sound::setListenerOrientation(const glm::vec3& forward, const glm::vec3& up) {
    if (!gDevice || !gContext) return;
    ALfloat ori[6] = {
        forward.x, forward.y, forward.z,
        up.x, up.y, up.z
    };
    alListenerfv(AL_ORIENTATION, ori);
}

void Sound::setListenerVelocity(const glm::vec3& vel) {
    if (!gDevice || !gContext) return;
    alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);
}

bool Sound::Footsteps::isPlaying() {
    return gFootstepRunning;
}

void Sound::Footsteps::end() {
    std::cout << gFootstepRunning << std::endl;
    gFootstepRunning = false;
}

void TQ_gStepClr(ALuint source) {
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);

    if (state != AL_PLAYING || !gFootstepRunning) {
        alDeleteSources(1, &source);
        gFootstepRunning = false;
    } else {
        syng::TaskQueue::Instance().enqueue([source]() {
            TQ_gStepClr(source);
        });
    }
}

void Sound::Footsteps::begin(const std::vector<WavData>& steps, const glm::vec3& pos, const glm::vec3& vel) {
    if (gFootstepRunning || steps.empty() || !gDevice || !gContext) return;

    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, steps.size() - 1);

    ALuint source;
    alGenSources(1, &source);

    alSourcei(source, AL_BUFFER, getALBuffer(steps[dist(rng)]));
    alSource3f(source, AL_POSITION, pos.x, pos.y, pos.z);
    alSource3f(source, AL_VELOCITY, vel.x, vel.y, vel.z);
    alSourcePlay(source);
    gFootstepRunning = true;

    syng::TaskQueue::Instance().enqueue([source]() {
        TQ_gStepClr(source);
    });
}