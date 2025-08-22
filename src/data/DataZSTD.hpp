#pragma once

#include "Syngine/serialization/DataSerializer.hpp"
#include <cstdint>
#include <filesystem>
#include <vector>

using namespace syng;

class DecoderDecompressor {
private:
    std::vector<uint8_t> buffer;
    int compressionLevel = 3;
    bool decompressed = false;
public:
    DecoderDecompressor(const std::vector<uint8_t>& buffer, int level = 3);
    DecoderDecompressor(const std::filesystem::path& path, int level = 3);

    void decompressZstd();
    DataDeserializer createWrapper();
};

class EncoderCompressor {
private:
    DataSerializer serializer;
    int compressionLevel = 3;
    bool compressed = false;
public:
    EncoderCompressor(const std::vector<uint8_t>& buffer, int level = 3);
    EncoderCompressor(const std::filesystem::path& path, int level = 3);
    EncoderCompressor(uint64_t max_size, int level = 3);

    DataSerializer& getWrapper();
    std::vector<uint8_t> compressZstd();
};

namespace GameData {
    std::vector<uint8_t> compressZstd(const std::vector<uint8_t>& data, int compressionLevel = 3);
    std::vector<uint8_t> decompressZstd(const std::vector<uint8_t>& compressed);

    void writeToFile(const std::filesystem::path& path, const std::vector<uint8_t>& buffer);
    std::vector<uint8_t> readFromFile(const std::filesystem::path& path);
}