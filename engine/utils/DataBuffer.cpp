#include "DataBuffer.h"
#include <stdexcept>

namespace nebula {

DataBuffer::DataBuffer()
    : m_pos(0)
{
    m_data.reserve(1024);
}

DataBuffer::DataBuffer(std::size_t initialCapacity)
    : m_pos(0)
{
    m_data.reserve(initialCapacity);
}

void DataBuffer::ensureCapacity(std::size_t needed) {
    std::size_t required = m_pos + needed;
    if (required > m_data.size()) {
        m_data.resize(required);
    }
}

void DataBuffer::write(const void* data, std::size_t size) {
    ensureCapacity(size);
    std::memcpy(m_data.data() + m_pos, data, size);
    m_pos += size;
}

void DataBuffer::read(void* data, std::size_t size) {
    if (m_pos + size > m_data.size()) {
        throw std::runtime_error("DataBuffer: not enough data to read");
    }
    std::memcpy(data, m_data.data() + m_pos, size);
    m_pos += size;
}

void DataBuffer::writeInt8(int8_t value) { write(&value, sizeof(value)); }
void DataBuffer::writeInt16(int16_t value) { write(&value, sizeof(value)); }
void DataBuffer::writeInt32(int32_t value) { write(&value, sizeof(value)); }
void DataBuffer::writeInt64(int64_t value) { write(&value, sizeof(value)); }
void DataBuffer::writeUInt8(uint8_t value) { write(&value, sizeof(value)); }
void DataBuffer::writeUInt16(uint16_t value) { write(&value, sizeof(value)); }
void DataBuffer::writeUInt32(uint32_t value) { write(&value, sizeof(value)); }
void DataBuffer::writeUInt64(uint64_t value) { write(&value, sizeof(value)); }
void DataBuffer::writeFloat(float value) { write(&value, sizeof(value)); }
void DataBuffer::writeDouble(double value) { write(&value, sizeof(value)); }
void DataBuffer::writeBool(bool value) { writeUInt8(value ? 1 : 0); }

void DataBuffer::writeString(const std::string& value) {
    writeUInt32(static_cast<uint32_t>(value.size()));
    write(value.data(), value.size());
}

void DataBuffer::writeBytes(const std::vector<uint8_t>& data) {
    write(data.data(), data.size());
}

int8_t DataBuffer::readInt8() { int8_t v; read(&v, sizeof(v)); return v; }
int16_t DataBuffer::readInt16() { int16_t v; read(&v, sizeof(v)); return v; }
int32_t DataBuffer::readInt32() { int32_t v; read(&v, sizeof(v)); return v; }
int64_t DataBuffer::readInt64() { int64_t v; read(&v, sizeof(v)); return v; }
uint8_t DataBuffer::readUInt8() { uint8_t v; read(&v, sizeof(v)); return v; }
uint16_t DataBuffer::readUInt16() { uint16_t v; read(&v, sizeof(v)); return v; }
uint32_t DataBuffer::readUInt32() { uint32_t v; read(&v, sizeof(v)); return v; }
uint64_t DataBuffer::readUInt64() { uint64_t v; read(&v, sizeof(v)); return v; }
float DataBuffer::readFloat() { float v; read(&v, sizeof(v)); return v; }
double DataBuffer::readDouble() { double v; read(&v, sizeof(v)); return v; }

bool DataBuffer::readBool() {
    return readUInt8() != 0;
}

std::string DataBuffer::readString() {
    uint32_t len = readUInt32();
    if (m_pos + len > m_data.size()) {
        throw std::runtime_error("DataBuffer: string exceeds data size");
    }
    std::string result(reinterpret_cast<const char*>(m_data.data() + m_pos), len);
    m_pos += len;
    return result;
}

std::vector<uint8_t> DataBuffer::readBytes(std::size_t size) {
    std::vector<uint8_t> result(size);
    read(result.data(), size);
    return result;
}

void DataBuffer::seek(std::size_t pos) {
    if (pos > m_data.size()) {
        m_data.resize(pos);
    }
    m_pos = pos;
}

void DataBuffer::skip(std::size_t count) {
    seek(m_pos + count);
}

void DataBuffer::reserve(std::size_t capacity) {
    m_data.reserve(capacity);
}

void DataBuffer::resize(std::size_t size) {
    m_data.resize(size);
    if (m_pos > size) m_pos = size;
}

void DataBuffer::clear() {
    m_data.clear();
    m_pos = 0;
}

void DataBuffer::reset() {
    m_pos = 0;
}

std::vector<uint8_t> DataBuffer::release() {
    std::vector<uint8_t> result = std::move(m_data);
    m_data.clear();
    m_pos = 0;
    return result;
}

DataBuffer& DataBuffer::operator<<(int32_t value) { writeInt32(value); return *this; }
DataBuffer& DataBuffer::operator>>(int32_t& value) { value = readInt32(); return *this; }
DataBuffer& DataBuffer::operator<<(float value) { writeFloat(value); return *this; }
DataBuffer& DataBuffer::operator>>(float& value) { value = readFloat(); return *this; }
DataBuffer& DataBuffer::operator<<(const std::string& value) { writeString(value); return *this; }
DataBuffer& DataBuffer::operator>>(std::string& value) { value = readString(); return *this; }

}

