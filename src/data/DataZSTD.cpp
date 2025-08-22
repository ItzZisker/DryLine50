#include "DataZSTD.hpp"
#include "Syngine/serialization/DataSerializer.hpp"

#include <fstream>
#include <vector>
#include <zstd.h>
#include <stdexcept>
#include <cstring>

DecoderDecompressor::DecoderDecompressor(const std::filesystem::path& path, int level) : compressionLevel(level), buffer(GameData::readFromFile(path)) {}

DecoderDecompressor::DecoderDecompressor(const std::vector<uint8_t>& buffer, int level) : buffer(buffer), compressionLevel(level) {}

DataDeserializer DecoderDecompressor::createWrapper() {
    if (!decompressed) throw std::runtime_error("Not decompressed yet!");
    return {buffer.data(), buffer.size()};
}

void DecoderDecompressor::decompressZstd() {
    if (decompressed) return;
    this->buffer = GameData::decompressZstd(buffer);
    this->decompressed = true;
}

EncoderCompressor::EncoderCompressor(const std::vector<uint8_t>& buffer, int level) : serializer(buffer.size()) {
    serializer.write(buffer.data(), buffer.size());
}

EncoderCompressor::EncoderCompressor(const std::filesystem::path& path, int level) : EncoderCompressor(GameData::readFromFile(path), level) {}

EncoderCompressor::EncoderCompressor(uint64_t max_size, int level) : serializer(max_size), compressionLevel(level) {}

std::vector<uint8_t> EncoderCompressor::compressZstd() {
    auto data = serializer.copyData();
    return GameData::compressZstd(data, compressionLevel);
}

DataSerializer& EncoderCompressor::getWrapper() {
    return this->serializer;
}

std::vector<uint8_t> GameData::compressZstd(const std::vector<uint8_t>& data, int compressionLevel) {
    size_t bound = ZSTD_compressBound(data.size());
    std::vector<uint8_t> compressed(bound);

    size_t cSize = ZSTD_compress(compressed.data(), bound, data.data(), data.size(), compressionLevel);
    if (ZSTD_isError(cSize)) {
        throw std::runtime_error(ZSTD_getErrorName(cSize));
    }
    compressed.resize(cSize);
    return compressed;
}

std::vector<uint8_t> GameData::decompressZstd(const std::vector<uint8_t>& compressed) {
    size_t originalSize = ZSTD_getFrameContentSize(compressed.data(), compressed.size());

    if (originalSize == ZSTD_CONTENTSIZE_UNKNOWN) {
        throw std::runtime_error("Original size unknown, cannot decompress safely.");
    } else if (originalSize == ZSTD_CONTENTSIZE_ERROR) {
        throw std::runtime_error("Invalid compressed data.");
    }

    std::vector<uint8_t> decompressed(originalSize);

    size_t dSize = ZSTD_decompress(decompressed.data(), originalSize, compressed.data(), compressed.size());
    if (ZSTD_isError(dSize)) throw std::runtime_error(ZSTD_getErrorName(dSize));

    return decompressed;
}

void GameData::writeToFile(const std::filesystem::path& path, const std::vector<uint8_t>& buffer) {
    std::ofstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(path, std::ios::binary);
    file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
    file.close();
}

std::vector<uint8_t> GameData::readFromFile(const std::filesystem::path& path) {
    std::ifstream file;
    file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    file.open(path, std::ios::binary);
    std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), {});
    file.close();
    return buffer;
}