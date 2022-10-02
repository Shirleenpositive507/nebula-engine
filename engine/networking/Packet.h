#pragma once

#include <SFML/Network/Packet.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include <type_traits>

namespace nebula {

    class Packet {
    public:
        Packet();

        void clear();
        bool endOfPacket() const;

        const void* getData() const;
        size_t getDataSize() const;

        template<typename T>
        void write(T value) {
            m_packet << value;
        }

        template<typename T>
        bool read(T& value) {
            return static_cast<bool>(m_packet >> value);
        }

        void writeString(const std::string& value);
        std::string readString();

        void writeVector2(float x, float y);
        void readVector2(float& x, float& y);

        void writeVector3(float x, float y, float z);
        void readVector3(float& x, float& y, float& z);

        void writeVector4(float x, float y, float z, float w);
        void readVector4(float& x, float& y, float& z, float& w);

        void writeInt8(int8_t value);
        int8_t readInt8();
        void writeInt16(int16_t value);
        int16_t readInt16();
        void writeInt32(int32_t value);
        int32_t readInt32();
        void writeInt64(int64_t value);
        int64_t readInt64();

        void writeFloat(float value);
        float readFloat();

        void writeBool(bool value);
        bool readBool();

        size_t getRemainingBytes() const;
        void reserve(size_t size);

        sf::Packet& getHandle() { return m_packet; }
        const sf::Packet& getHandle() const { return m_packet; }

    private:
        sf::Packet m_packet;
    };

    class PacketPool {
    public:
        PacketPool(size_t initialSize = 64);

        Packet* acquire();
        void release(Packet* packet);

        size_t getActiveCount() const { return m_activeCount; }
        size_t getPoolSize() const { return m_pool.size(); }

        void clear();

    private:
        std::vector<Packet> m_pool;
        std::vector<Packet*> m_freeList;
        size_t m_activeCount = 0;
    };

}
