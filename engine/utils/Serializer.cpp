#include "Serializer.h"

namespace nebula {

    Serializer::Serializer(Endianness targetEndian)
        : m_targetEndian(targetEndian) {}

    template<typename T>
    void Serializer::writeValue(T value) {
        if (m_targetEndian != Endianness::Native && m_targetEndian != getNativeEndianness()) {
            if constexpr (sizeof(T) == 2) value = static_cast<T>(swapEndian16(static_cast<uint16_t>(value)));
            else if constexpr (sizeof(T) == 4) value = static_cast<T>(swapEndian32(static_cast<uint32_t>(value)));
            else if constexpr (sizeof(T) == 8) value = static_cast<T>(swapEndian64(static_cast<uint64_t>(value)));
        }
        size_t needed = m_pos + sizeof(T);
        if (needed > m_data.size()) m_data.resize(needed);
        std::memcpy(m_data.data() + m_pos, &value, sizeof(T));
        m_pos += sizeof(T);
    }

    void Serializer::writeInt8(int8_t value) { writeValue(value); }
    void Serializer::writeInt16(int16_t value) { writeValue(value); }
    void Serializer::writeInt32(int32_t value) { writeValue(value); }
    void Serializer::writeInt64(int64_t value) { writeValue(value); }
    void Serializer::writeUInt8(uint8_t value) { writeValue(value); }
    void Serializer::writeUInt16(uint16_t value) { writeValue(value); }
    void Serializer::writeUInt32(uint32_t value) { writeValue(value); }
    void Serializer::writeUInt64(uint64_t value) { writeValue(value); }

    void Serializer::writeFloat(float value) { writeValue(value); }
    void Serializer::writeDouble(double value) { writeValue(value); }

    void Serializer::writeBool(bool value) {
        writeUInt8(value ? 1 : 0);
    }

    void Serializer::writeString(const std::string& value) {
        writeUInt32(static_cast<uint32_t>(value.size()));
        writeBytes(value.data(), value.size());
    }

    void Serializer::writeBytes(const void* data, size_t size) {
        size_t needed = m_pos + size;
        if (needed > m_data.size()) m_data.resize(needed);
        std::memcpy(m_data.data() + m_pos, data, size);
        m_pos += size;
    }

    void Serializer::writeBytes(const std::vector<uint8_t>& data) {
        writeBytes(data.data(), data.size());
    }

    void Serializer::clear() {
        m_data.clear();
        m_pos = 0;
    }

    void Serializer::seek(size_t pos) {
        m_pos = pos;
    }

    Deserializer::Deserializer(const std::vector<uint8_t>& data, Endianness dataEndian)
        : m_data(data), m_dataEndian(dataEndian) {}

    template<typename T>
    T Deserializer::readValue() {
        if (m_pos + sizeof(T) > m_data.size()) {
            throw std::runtime_error("Deserializer: unexpected end of data");
        }
        T value;
        std::memcpy(&value, m_data.data() + m_pos, sizeof(T));
        m_pos += sizeof(T);
        if (m_dataEndian != Endianness::Native && m_dataEndian != getNativeEndianness()) {
            if constexpr (sizeof(T) == 2) value = static_cast<T>(swapEndian16(static_cast<uint16_t>(value)));
            else if constexpr (sizeof(T) == 4) value = static_cast<T>(swapEndian32(static_cast<uint32_t>(value)));
            else if constexpr (sizeof(T) == 8) value = static_cast<T>(swapEndian64(static_cast<uint64_t>(value)));
        }
        return value;
    }

    int8_t Deserializer::readInt8() { return readValue<int8_t>(); }
    int16_t Deserializer::readInt16() { return readValue<int16_t>(); }
    int32_t Deserializer::readInt32() { return readValue<int32_t>(); }
    int64_t Deserializer::readInt64() { return readValue<int64_t>(); }
    uint8_t Deserializer::readUInt8() { return readValue<uint8_t>(); }
    uint16_t Deserializer::readUInt16() { return readValue<uint16_t>(); }
    uint32_t Deserializer::readUInt32() { return readValue<uint32_t>(); }
    uint64_t Deserializer::readUInt64() { return readValue<uint64_t>(); }
    float Deserializer::readFloat() { return readValue<float>(); }
    double Deserializer::readDouble() { return readValue<double>(); }

    bool Deserializer::readBool() {
        return readUInt8() != 0;
    }

    std::string Deserializer::readString() {
        uint32_t len = readUInt32();
        if (m_pos + len > m_data.size()) {
            throw std::runtime_error("Deserializer: string exceeds data size");
        }
        std::string result(reinterpret_cast<const char*>(m_data.data() + m_pos), len);
        m_pos += len;
        return result;
    }

    void Deserializer::readBytes(void* data, size_t size) {
        if (m_pos + size > m_data.size()) {
            throw std::runtime_error("Deserializer: not enough data");
        }
        std::memcpy(data, m_data.data() + m_pos, size);
        m_pos += size;
    }

    std::vector<uint8_t> Deserializer::readBytes(size_t size) {
        std::vector<uint8_t> result(size);
        readBytes(result.data(), size);
        return result;
    }

    void Deserializer::seek(size_t pos) {
        m_pos = pos;
    }

    Archive::Archive(uint32_t version)
        : m_version(version), m_loading(false)
    {
        m_serializer = std::make_unique<Serializer>();
    }

    Archive::Archive(uint32_t version, bool loading, const std::vector<uint8_t>* data)
        : m_version(version), m_loading(loading)
    {
        if (loading) {
            m_deserializer = std::make_unique<Deserializer>(*data);
        } else {
            m_serializer = std::make_unique<Serializer>();
        }
    }

    Archive Archive::createReader(const std::vector<uint8_t>& data, uint32_t version) {
        return Archive(version, true, &data);
    }

    Archive Archive::createWriter(uint32_t version) {
        return Archive(version);
    }

    std::vector<uint8_t> Archive::getData() const {
        if (m_serializer) return m_serializer->getData();
        return {};
    }

    template<typename T>
    void Archive::serialize(T& value) {
        static_assert(std::is_arithmetic_v<T>, "Archive::serialize only supports arithmetic types, use overloads");
        if (m_loading) {
            value = m_deserializer->readValue<T>();
        } else {
            m_serializer->writeValue(value);
        }
    }

    void Archive::serialize(int8_t& value) { if (m_loading) value = m_deserializer->readInt8(); else m_serializer->writeInt8(value); }
    void Archive::serialize(int16_t& value) { if (m_loading) value = m_deserializer->readInt16(); else m_serializer->writeInt16(value); }
    void Archive::serialize(int32_t& value) { if (m_loading) value = m_deserializer->readInt32(); else m_serializer->writeInt32(value); }
    void Archive::serialize(int64_t& value) { if (m_loading) value = m_deserializer->readInt64(); else m_serializer->writeInt64(value); }
    void Archive::serialize(uint8_t& value) { if (m_loading) value = m_deserializer->readUInt8(); else m_serializer->writeUInt8(value); }
    void Archive::serialize(uint16_t& value) { if (m_loading) value = m_deserializer->readUInt16(); else m_serializer->writeUInt16(value); }
    void Archive::serialize(uint32_t& value) { if (m_loading) value = m_deserializer->readUInt32(); else m_serializer->writeUInt32(value); }
    void Archive::serialize(uint64_t& value) { if (m_loading) value = m_deserializer->readUInt64(); else m_serializer->writeUInt64(value); }
    void Archive::serialize(float& value) { if (m_loading) value = m_deserializer->readFloat(); else m_serializer->writeFloat(value); }
    void Archive::serialize(double& value) { if (m_loading) value = m_deserializer->readDouble(); else m_serializer->writeDouble(value); }
    void Archive::serialize(bool& value) { if (m_loading) value = m_deserializer->readBool(); else m_serializer->writeBool(value); }

    void Archive::serialize(std::string& value) {
        if (m_loading) value = m_deserializer->readString();
        else m_serializer->writeString(value);
    }

    void Archive::serialize(std::vector<uint8_t>& value) {
        uint32_t size = static_cast<uint32_t>(value.size());
        serialize(size);
        if (m_loading) {
            value = m_deserializer->readBytes(size);
        } else {
            m_serializer->writeBytes(value);
        }
    }

}
