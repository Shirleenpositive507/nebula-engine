#pragma once

#include <vector>
#include <cstdint>
#include <string>

namespace nebula {

enum class CompressionLevel {
    Fastest,
    Default,
    Best
};

struct CompressedBlock {
    std::vector<uint8_t> data;
    std::size_t originalSize;
    std::size_t compressedSize;
    bool valid;
};

class RLECompressor {
public:
    static std::vector<uint8_t> compress(const void* data, std::size_t size);
    static std::vector<uint8_t> decompress(const void* data, std::size_t size);

    static std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);
};

class LZ4Compressor {
public:
    static std::vector<uint8_t> compress(const void* data, std::size_t size, CompressionLevel level = CompressionLevel::Default);
    static std::vector<uint8_t> decompress(const void* data, std::size_t size, std::size_t originalSize);

    static std::vector<uint8_t> compress(const std::vector<uint8_t>& data, CompressionLevel level = CompressionLevel::Default);
    static std::vector<uint8_t> decompress(const std::vector<uint8_t>& data, std::size_t originalSize);
};

class HuffmanCompressor {
public:
    static std::vector<uint8_t> compress(const void* data, std::size_t size);
    static std::vector<uint8_t> decompress(const void* data, std::size_t size);

    static std::vector<uint8_t> compress(const std::vector<uint8_t>& data);
    static std::vector<uint8_t> decompress(const std::vector<uint8_t>& data);
};

class StreamingCompressor {
public:
    StreamingCompressor(CompressionLevel level = CompressionLevel::Default);
    ~StreamingCompressor();

    void write(const void* data, std::size_t size);
    std::vector<uint8_t> flush();
    void reset();

    std::size_t getBytesWritten() const { return m_bytesWritten; }
    std::size_t getBytesCompressed() const { return m_bytesCompressed; }

private:
    CompressionLevel m_level;
    std::vector<uint8_t> m_buffer;
    std::size_t m_bytesWritten;
    std::size_t m_bytesCompressed;
};

class CompressionUtils {
public:
    static CompressedBlock compress(const void* data, std::size_t size, CompressionLevel level = CompressionLevel::Default);
    static std::vector<uint8_t> decompress(const CompressedBlock& block);

    static double getCompressionRatio(std::size_t original, std::size_t compressed);
    static bool isCompressed(const void* data, std::size_t size);
};

}

