#pragma once

#include <vector>
#include <cstdint>
#include <chrono>
#include <functional>

namespace nebula {

    static constexpr size_t NETMESSAGE_HEADER_SIZE = 12;

    enum class MessageType : uint8_t {
        Connect = 0,
        Disconnect = 1,
        Data = 2,
        Acknowledge = 3,
        Ping = 4,
        Pong = 5,
        Custom = 6
    };

    class NetworkMessage {
    public:
        NetworkMessage();

        void setType(MessageType type);
        MessageType getType() const;

        void setID(uint32_t id);
        uint32_t getID() const;

        void setPayload(const std::vector<uint8_t>& data);
        void setPayload(const uint8_t* data, size_t size);
        const std::vector<uint8_t>& getPayload() const;

        uint64_t getTimestamp() const;
        size_t getSize() const;

        std::vector<uint8_t> serialize() const;
        bool deserialize(const std::vector<uint8_t>& buffer);

        static NetworkMessage createACK(uint32_t ackID);
        static NetworkMessage createPing();
        static NetworkMessage createPong();

        void reset();

    private:
        MessageType m_type;
        uint32_t m_id;
        std::vector<uint8_t> m_payload;
        uint64_t m_timestamp;
    };

    class MessagePool {
    public:
        MessagePool(size_t initialSize = 64);

        NetworkMessage* acquire();
        void release(NetworkMessage* msg);

        size_t getActiveCount() const { return m_activeCount; }
        size_t getPoolSize() const { return m_pool.size(); }

        void clear();

    private:
        std::vector<NetworkMessage> m_pool;
        std::vector<NetworkMessage*> m_freeList;
        size_t m_activeCount = 0;
    };

}
