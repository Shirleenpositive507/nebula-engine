#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <memory>
#include "Hashing.h"
#include "Compression.h"

namespace nebula {

    enum class Endianness {
        Little,
        Big,
        Native
    };

    inline Endianness getNativeEndianness() {
        uint32_t test = 1;
        return (reinterpret_cast<char*>(&test)[0] == 1) ? Endianness::Little : Endianness::Big;
    }

    inline uint16_t swapEndian16(uint16_t v) {
        return ((v >> 8) & 0xFF) | ((v & 0xFF) << 8);
    }

    inline uint32_t swapEndian32(uint32_t v) {
        return ((v >> 24) & 0xFF) | ((v >> 8) & 0xFF00) | ((v & 0xFF00) << 8) | ((v & 0xFF) << 24);
    }

    inline uint64_t swapEndian64(uint64_t v) {
        return ((v >> 56) & 0xFF) | ((v >> 40) & 0xFF00) | ((v >> 24) & 0xFF0000) |
               ((v >> 8) & 0xFF000000) | ((v & 0xFF000000) << 8) | ((v & 0xFF0000) << 24) |
               ((v & 0xFF00) << 40) | ((v & 0xFF) << 56);
    }

    class Serializer {
    public:
        Serializer(Endianness targetEndian = Endianness::Native);

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
        void writeBytes(const void* data, size_t size);
        void writeBytes(const std::vector<uint8_t>& data);

        const std::vector<uint8_t>& getData() const { return m_data; }
        size_t getSize() const { return m_data.size(); }
        void clear();
        void seek(size_t pos);
        size_t tell() const { return m_pos; }

        Endianness getEndianness() const { return m_targetEndian; }
        void setEndianness(Endianness endian) { m_targetEndian = endian; }

    private:
        std::vector<uint8_t> m_data;
        size_t m_pos = 0;
        Endianness m_targetEndian;

        template<typename T>
        void writeValue(T value);
    };

    class Deserializer {
    public:
        Deserializer(const std::vector<uint8_t>& data, Endianness dataEndian = Endianness::Native);

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
        void readBytes(void* data, size_t size);
        std::vector<uint8_t> readBytes(size_t size);

        size_t getRemaining() const { return m_data.size() - m_pos; }
        size_t getSize() const { return m_data.size(); }
        size_t tell() const { return m_pos; }
        void seek(size_t pos);
        bool isEnd() const { return m_pos >= m_data.size(); }

        Endianness getEndianness() const { return m_dataEndian; }
        void setEndianness(Endianness endian) { m_dataEndian = endian; }

    private:
        const std::vector<uint8_t>& m_data;
        size_t m_pos = 0;
        Endianness m_dataEndian;

        template<typename T>
        T readValue();
    };

    class Archive {
    public:
        Archive(uint32_t version = 0);

        uint32_t getVersion() const { return m_version; }
        void setVersion(uint32_t version) { m_version = version; }
        bool isVersion(uint32_t version) const { return m_version >= version; }

        template<typename T>
        void serialize(T& value);

        void serialize(int8_t& value);
        void serialize(int16_t& value);
        void serialize(int32_t& value);
        void serialize(int64_t& value);
        void serialize(uint8_t& value);
        void serialize(uint16_t& value);
        void serialize(uint32_t& value);
        void serialize(uint64_t& value);
        void serialize(float& value);
        void serialize(double& value);
        void serialize(bool& value);
        void serialize(std::string& value);
        void serialize(std::vector<uint8_t>& value);

        bool isLoading() const { return m_loading; }
        bool isStoring() const { return !m_loading; }

        static Archive createReader(const std::vector<uint8_t>& data, uint32_t version = 0);
        static Archive createWriter(uint32_t version = 0);

        std::vector<uint8_t> getData() const;

        void setUseChecksum(bool enabled) { m_useChecksum = enabled; }
        bool isChecksumEnabled() const { return m_useChecksum; }

        void setCompressionEnabled(bool enabled) { m_compressionEnabled = enabled; }
        bool isCompressionEnabled() const { return m_compressionEnabled; }

        void setCompressionLevel(CompressionLevel level) { m_compressionLevel = level; }
        CompressionLevel getCompressionLevel() const { return m_compressionLevel; }

        uint32_t computeChecksum() const;
        bool verifyChecksum(uint32_t checksum) const;

        std::vector<uint8_t> getCompressed() const;
        bool loadCompressed(const std::vector<uint8_t>& compressedData);

    private:
        uint32_t m_version;
        bool m_loading;

        std::unique_ptr<Serializer> m_serializer;
        std::unique_ptr<Deserializer> m_deserializer;
        bool m_useChecksum;
        bool m_compressionEnabled;
        CompressionLevel m_compressionLevel;

        Archive(uint32_t version, bool loading, const std::vector<uint8_t>* data);
    };

}
