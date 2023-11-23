#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <string>
#include <algorithm>

namespace nebula {

class DataBuffer {
public:
    DataBuffer();
    explicit DataBuffer(std::size_t initialCapacity);

    void write(const void* data, std::size_t size);
    void read(void* data, std::size_t size);

    void writeInt8(int8_t value);
    void writeInt16(int16_t value);
    void writeInt32(int32_t value);
    void writeInt64(int64_t value);
    void writeUInt8(uint8_t value);
    void writeUInt16(uint16_t value);
    void writeUInt32(uint32_t value);
    void writeUInt64(uint64_t value);
    void writeFloat(float value);
    void writeDouble(double value);
    void writeBool(bool value);
    void writeString(const std::string& value);
    void writeBytes(const std::vector<uint8_t>& data);

    int8_t readInt8();
    int16_t readInt16();
    int32_t readInt32();
    int64_t readInt64();
    uint8_t readUInt8();
    uint16_t readUInt16();
    uint32_t readUInt32();
    uint64_t readUInt64();
    float readFloat();
    double readDouble();
    bool readBool();
    std::string readString();
    std::vector<uint8_t> readBytes(std::size_t size);

    void seek(std::size_t pos);
    void skip(std::size_t count);
    std::size_t tell() const { return m_pos; }
    std::size_t size() const { return m_data.size(); }
    std::size_t capacity() const { return m_data.capacity(); }
    std::size_t remaining() const { return m_data.size() - m_pos; }
    bool isEnd() const { return m_pos >= m_data.size(); }

    void reserve(std::size_t capacity);
    void resize(std::size_t size);
    void clear();
    void reset();

    const uint8_t* data() const { return m_data.data(); }
    uint8_t* data() { return m_data.data(); }
    const std::vector<uint8_t>& getVector() const { return m_data; }
    std::vector<uint8_t> release();

    DataBuffer& operator<<(int32_t value);
    DataBuffer& operator>>(int32_t& value);
    DataBuffer& operator<<(float value);
    DataBuffer& operator>>(float& value);
    DataBuffer& operator<<(const std::string& value);
    DataBuffer& operator>>(std::string& value);

private:
    void ensureCapacity(std::size_t needed);

    std::vector<uint8_t> m_data;
    std::size_t m_pos;
};

}

