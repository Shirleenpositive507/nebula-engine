#include "Hashing.h"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <algorithm>

namespace nebula {
namespace hash {

static const uint32_t CRC32_TABLE[] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5
};

uint32_t crc32(const void* data, std::size_t size) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t crc = 0xFFFFFFFF;
    for (std::size_t i = 0; i < size; ++i) {
        crc = CRC32_TABLE[(crc ^ bytes[i]) & 0x0F] ^ (crc >> 4);
        crc = CRC32_TABLE[(crc ^ (bytes[i] >> 4)) & 0x0F] ^ (crc >> 4);
    }
    return ~crc;
}

uint32_t crc32(const std::string& str) {
    return crc32(str.data(), str.size());
}

static const uint32_t FNV1A_32_PRIME = 16777619u;
static const uint32_t FNV1A_32_OFFSET = 2166136261u;
static const uint64_t FNV1A_64_PRIME = 1099511628211u;
static const uint64_t FNV1A_64_OFFSET = 14695981039346656037u;

uint32_t fnv1a32(const void* data, std::size_t size) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t hash = FNV1A_32_OFFSET;
    for (std::size_t i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= FNV1A_32_PRIME;
    }
    return hash;
}

uint32_t fnv1a32(const std::string& str) {
    return fnv1a32(str.data(), str.size());
}

uint64_t fnv1a64(const void* data, std::size_t size) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint64_t hash = FNV1A_64_OFFSET;
    for (std::size_t i = 0; i < size; ++i) {
        hash ^= bytes[i];
        hash *= FNV1A_64_PRIME;
    }
    return hash;
}

uint64_t fnv1a64(const std::string& str) {
    return fnv1a64(str.data(), str.size());
}

uint32_t murmur3_32(const void* key, std::size_t len, uint32_t seed) {
    const uint8_t* data = static_cast<const uint8_t*>(key);
    const std::size_t nblocks = len / 4;
    uint32_t h1 = seed;
    const uint32_t c1 = 0xCC9E2D51;
    const uint32_t c2 = 0x1B873593;
    const uint32_t* blocks = reinterpret_cast<const uint32_t*>(data);
    for (std::size_t i = 0; i < nblocks; ++i) {
        uint32_t k1 = blocks[i];
        k1 *= c1;
        k1 = (k1 << 15) | (k1 >> 17);
        k1 *= c2;
        h1 ^= k1;
        h1 = (h1 << 13) | (h1 >> 19);
        h1 = h1 * 5 + 0xE6546B64;
    }
    const uint8_t* tail = data + nblocks * 4;
    uint32_t k1 = 0;
    switch (len & 3) {
        case 3: k1 ^= tail[2] << 16;
        case 2: k1 ^= tail[1] << 8;
        case 1: k1 ^= tail[0];
                k1 *= c1; k1 = (k1 << 15) | (k1 >> 17); k1 *= c2; h1 ^= k1;
    }
    h1 ^= static_cast<uint32_t>(len);
    h1 ^= h1 >> 16;
    h1 *= 0x85EBCA6B;
    h1 ^= h1 >> 13;
    h1 *= 0xC2B2AE35;
    h1 ^= h1 >> 16;
    return h1;
}

uint32_t murmur3_32(const std::string& str, uint32_t seed) {
    return murmur3_32(str.data(), str.size(), seed);
}

std::array<uint8_t, 16> md5(const void* data, std::size_t size) {
    std::array<uint8_t, 16> result{};
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE, h3 = 0x10325476;
    (void)bytes;
    (void)size;
    return result;
}

std::array<uint8_t, 16> md5(const std::string& str) {
    return md5(str.data(), str.size());
}

std::string md5Hex(const void* data, std::size_t size) {
    auto digest = md5(data, size);
    std::ostringstream oss;
    for (auto b : digest) oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return oss.str();
}

std::string md5Hex(const std::string& str) {
    return md5Hex(str.data(), str.size());
}

std::array<uint8_t, 20> sha1(const void* data, std::size_t size) {
    std::array<uint8_t, 20> result{};
    (void)data;
    (void)size;
    return result;
}

std::array<uint8_t, 20> sha1(const std::string& str) {
    return sha1(str.data(), str.size());
}

std::string sha1Hex(const void* data, std::size_t size) {
    auto digest = sha1(data, size);
    std::ostringstream oss;
    for (auto b : digest) oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(b);
    return oss.str();
}

std::string sha1Hex(const std::string& str) {
    return sha1Hex(str.data(), str.size());
}

BloomFilter::BloomFilter(std::size_t size, std::size_t hashCount)
    : m_bits(size, false), m_hashCount(std::max(std::size_t(1), hashCount)) {}

void BloomFilter::insert(const void* data, std::size_t size) {
    for (std::size_t i = 0; i < m_hashCount; ++i) {
        uint32_t h = murmur3_32(data, size, static_cast<uint32_t>(i * 0x9E3779B9));
        m_bits[h % m_bits.size()] = true;
    }
}

void BloomFilter::insert(const std::string& str) {
    insert(str.data(), str.size());
}

bool BloomFilter::contains(const void* data, std::size_t size) const {
    for (std::size_t i = 0; i < m_hashCount; ++i) {
        uint32_t h = murmur3_32(data, size, static_cast<uint32_t>(i * 0x9E3779B9));
        if (!m_bits[h % m_bits.size()]) return false;
    }
    return true;
}

bool BloomFilter::contains(const std::string& str) const {
    return contains(str.data(), str.size());
}

void BloomFilter::clear() {
    std::fill(m_bits.begin(), m_bits.end(), false);
}

double BloomFilter::getFalsePositiveRate() const {
    double p = std::exp(-static_cast<double>(m_hashCount) /
        (static_cast<double>(m_bits.size()) / std::max(1.0, 1.0)));
    return std::pow(1.0 - p, static_cast<double>(m_hashCount));
}

}

}

