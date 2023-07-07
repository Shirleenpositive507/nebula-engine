#pragma once

#include <SFML/Network/Packet.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include <type_traits>
#include <unordered_map>
#include <functional>
#include <memory>

namespace nebula {

    struct PacketFragment {
        uint32_t packetId;
        uint16_t fragmentIndex;
        uint16_t totalFragments;
        uint16_t fragmentSize;
        std::vector<uint8_t> data;
    };

    class Packet {
    public:
        Packet();

        void clear();
        bool endOfPacket() const;

        const void* getData() const;
        size_t getDataSize() const;

        bool isFragmented() const { return m_fragmented; }
        uint32_t getPacketId() const { return m_packetId; }

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

        void setSequenceNumber(uint16_t seq);
        uint16_t getSequenceNumber() const;

        void setChecksum(uint32_t crc);
        uint32_t getChecksum() const;
        bool verifyChecksum() const;
        uint32_t calculateCRC32() const;

        void compress();
        void decompress();

        void encrypt(const uint8_t* key, size_t keyLength);
        void decrypt(const uint8_t* key, size_t keyLength);

        std::vector<Packet> fragment(size_t maxFragmentSize);
        bool reassemble(const std::vector<Packet>& fragments);
        bool addFragment(const PacketFragment& fragment);
        bool isReassemblyComplete() const;

        sf::Packet& getHandle() { return m_packet; }
        const sf::Packet& getHandle() const { return m_packet; }

    private:
        void writeHeader();
        void readHeader();

        sf::Packet m_packet;
        uint16_t m_sequenceNumber;
        uint32_t m_checksum;
        bool m_fragmented;
        uint32_t m_packetId;
        std::vector<PacketFragment> m_fragments;
        static uint32_t s_nextPacketId;
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

    class PacketFragmenter {
    public:
        PacketFragmenter(size_t maxFragmentSize = 1024);

        std::vector<PacketFragment> fragment(const Packet& packet);
        bool addFragment(const PacketFragment& fragment);
        Packet reassemble(uint32_t packetId);
        bool isComplete(uint32_t packetId) const;
        void clear(uint32_t packetId);

    private:
        size_t m_maxFragmentSize;
        struct ReassemblyBuffer {
            std::vector<PacketFragment> fragments;
            uint16_t totalFragments;
            uint64_t lastReceiveTime;
        };
        std::unordered_map<uint32_t, ReassemblyBuffer> m_reassemblyBuffers;
        static constexpr uint64_t FRAGMENT_TIMEOUT_MS = 5000;
    };

}
