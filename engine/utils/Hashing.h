#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <array>
#include <cstring>

namespace nebula {

namespace hash {

uint32_t crc32(const void* data, std::size_t size);
uint32_t crc32(const std::string& str);

std::array<uint8_t, 16> md5(const void* data, std::size_t size);
std::array<uint8_t, 16> md5(const std::string& str);
std::string md5Hex(const void* data, std::size_t size);
std::string md5Hex(const std::string& str);

std::array<uint8_t, 20> sha1(const void* data, std::size_t size);
std::array<uint8_t, 20> sha1(const std::string& str);
std::string sha1Hex(const void* data, std::size_t size);
std::string sha1Hex(const std::string& str);

uint32_t fnv1a32(const void* data, std::size_t size);
uint32_t fnv1a32(const std::string& str);
uint64_t fnv1a64(const void* data, std::size_t size);
uint64_t fnv1a64(const std::string& str);

uint32_t murmur3_32(const void* key, std::size_t len, uint32_t seed = 0);
uint32_t murmur3_32(const std::string& str, uint32_t seed = 0);

constexpr uint32_t constexprHash(const char* str, std::size_t len) {
    uint32_t hash = 2166136261u;
    for (std::size_t i = 0; i < len; ++i) {
        hash ^= static_cast<uint8_t>(str[i]);
        hash *= 16777619u;
    }
    return hash;
}

constexpr uint32_t operator"" _hash(const char* str, std::size_t len) {
    return constexprHash(str, len);
}

template <typename T>
inline uint32_t hashBytes(const T& value) {
    return fnv1a32(&value, sizeof(T));
}

template <>
inline uint32_t hashBytes<std::string>(const std::string& value) {
    return fnv1a32(value);
}

class BloomFilter {
public:
    BloomFilter(std::size_t size = 1024, std::size_t hashCount = 3);

    void insert(const void* data, std::size_t size);
    void insert(const std::string& str);
    bool contains(const void* data, std::size_t size) const;
    bool contains(const std::string& str) const;
    void clear();
    std::size_t getSize() const { return m_bits.size(); }
    double getFalsePositiveRate() const;

private:
    std::vector<bool> m_bits;
    std::size_t m_hashCount;
};

}

}

