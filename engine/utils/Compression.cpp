#include "Compression.h"
#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace nebula {

std::vector<uint8_t> RLECompressor::compress(const void* data, std::size_t size) {
    const uint8_t* input = static_cast<const uint8_t*>(data);
    std::vector<uint8_t> output;
    output.reserve(size);

    for (std::size_t i = 0; i < size;) {
        uint8_t current = input[i];
        std::size_t run = 1;
        while (i + run < size && run < 255 && input[i + run] == current) {
            ++run;
        }

        if (run >= 3) {
            output.push_back(static_cast<uint8_t>(run));
            output.push_back(current);
            i += run;
        } else {
            std::size_t literalStart = i;
            std::size_t literalEnd = i;
            while (literalEnd < size && (literalEnd - literalStart) < 128) {
                std::size_t nextRun = 1;
                while (literalEnd + nextRun < size && nextRun < 255 &&
                       input[literalEnd] == input[literalEnd + nextRun]) {
                    ++nextRun;
                }
                if (nextRun >= 3) break;
                ++literalEnd;
            }
            std::size_t literalLen = literalEnd - literalStart;
            output.push_back(static_cast<uint8_t>(literalLen | 0x80));
            for (std::size_t j = 0; j < literalLen; ++j) {
                output.push_back(input[literalStart + j]);
            }
            i = literalEnd;
        }
    }

    return output;
}

std::vector<uint8_t> RLECompressor::decompress(const void* data, std::size_t size) {
    const uint8_t* input = static_cast<const uint8_t*>(data);
    std::vector<uint8_t> output;

    for (std::size_t i = 0; i < size;) {
        uint8_t header = input[i++];
        if (header & 0x80) {
            std::size_t literalLen = header & 0x7F;
            for (std::size_t j = 0; j < literalLen && i < size; ++j) {
                output.push_back(input[i++]);
            }
        } else {
            std::size_t run = header;
            if (i < size) {
                uint8_t value = input[i++];
                for (std::size_t j = 0; j < run; ++j) {
                    output.push_back(value);
                }
            }
        }
    }

    return output;
}

std::vector<uint8_t> RLECompressor::compress(const std::vector<uint8_t>& data) {
    return compress(data.data(), data.size());
}

std::vector<uint8_t> RLECompressor::decompress(const std::vector<uint8_t>& data) {
    return decompress(data.data(), data.size());
}

std::vector<uint8_t> LZ4Compressor::compress(const void* data, std::size_t size, CompressionLevel level) {
    (void)level;
    const uint8_t* input = static_cast<const uint8_t*>(data);
    std::vector<uint8_t> output;
    output.reserve(size);

    std::size_t ip = 0;
    while (ip < size) {
        std::size_t matchStart = 0;
        std::size_t matchLen = 0;
        std::size_t maxMatch = std::min(size - ip, std::size_t(255 + 18));

        for (std::size_t j = (ip > 65535) ? ip - 65535 : 0; j < ip; ++j) {
            std::size_t len = 0;
            while (len < maxMatch && input[j + len] == input[ip + len]) {
                ++len;
            }
            if (len > matchLen) {
                matchLen = len;
                matchStart = j;
                if (len == maxMatch) break;
            }
        }

        if (matchLen >= 4) {
            std::size_t literalLen = ip - matchStart;
            output.push_back(static_cast<uint8_t>(std::min(literalLen, std::size_t(15))));
            for (std::size_t k = 0; k < std::min(literalLen, std::size_t(15)); ++k) {
                output.push_back(input[matchStart + k]);
            }
            ip += matchLen;
        } else {
            output.push_back(0);
            output.push_back(input[ip++]);
        }
    }

    return output;
}

std::vector<uint8_t> LZ4Compressor::decompress(const void* data, std::size_t size, std::size_t originalSize) {
    (void)size;
    const uint8_t* input = static_cast<const uint8_t*>(data);
    std::vector<uint8_t> output;
    output.reserve(originalSize);
    (void)input;
    return output;
}

std::vector<uint8_t> LZ4Compressor::compress(const std::vector<uint8_t>& data, CompressionLevel level) {
    return compress(data.data(), data.size(), level);
}

std::vector<uint8_t> LZ4Compressor::decompress(const std::vector<uint8_t>& data, std::size_t originalSize) {
    return decompress(data.data(), data.size(), originalSize);
}

std::vector<uint8_t> HuffmanCompressor::compress(const void* data, std::size_t size) {
    (void)data;
    (void)size;
    return std::vector<uint8_t>();
}

std::vector<uint8_t> HuffmanCompressor::decompress(const void* data, std::size_t size) {
    (void)data;
    (void)size;
    return std::vector<uint8_t>();
}

std::vector<uint8_t> HuffmanCompressor::compress(const std::vector<uint8_t>& data) {
    return compress(data.data(), data.size());
}

std::vector<uint8_t> HuffmanCompressor::decompress(const std::vector<uint8_t>& data) {
    return decompress(data.data(), data.size());
}

StreamingCompressor::StreamingCompressor(CompressionLevel level)
    : m_level(level)
    , m_bytesWritten(0)
    , m_bytesCompressed(0)
{
    m_buffer.reserve(65536);
}

StreamingCompressor::~StreamingCompressor() {}

void StreamingCompressor::write(const void* data, std::size_t size) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    m_buffer.insert(m_buffer.end(), bytes, bytes + size);
    m_bytesWritten += size;
}

std::vector<uint8_t> StreamingCompressor::flush() {
    auto compressed = LZ4Compressor::compress(m_buffer, m_level);
    m_bytesCompressed += compressed.size();
    m_buffer.clear();
    return compressed;
}

void StreamingCompressor::reset() {
    m_buffer.clear();
    m_bytesWritten = 0;
    m_bytesCompressed = 0;
}

CompressedBlock CompressionUtils::compress(const void* data, std::size_t size, CompressionLevel level) {
    CompressedBlock block;
    block.originalSize = size;
    auto compressed = LZ4Compressor::compress(data, size, level);
    block.data = compressed;
    block.compressedSize = compressed.size();
    block.valid = true;
    return block;
}

std::vector<uint8_t> CompressionUtils::decompress(const CompressedBlock& block) {
    if (!block.valid) return {};
    return LZ4Compressor::decompress(block.data, block.originalSize);
}

double CompressionUtils::getCompressionRatio(std::size_t original, std::size_t compressed) {
    if (compressed == 0) return 0.0;
    return static_cast<double>(original) / static_cast<double>(compressed);
}

bool CompressionUtils::isCompressed(const void* data, std::size_t size) {
    (void)data;
    (void)size;
    return false;
}

}

