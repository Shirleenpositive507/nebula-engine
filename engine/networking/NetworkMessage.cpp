#include "NetworkMessage.h"
#include <cstring>

namespace nebula {

    NetworkMessage::NetworkMessage()
        : m_type(MessageType::Data)
        , m_id(0)
        , m_timestamp(0)
    {
    }

    void NetworkMessage::setType(MessageType type) {
        m_type = type;
    }

    MessageType NetworkMessage::getType() const {
        return m_type;
    }

    void NetworkMessage::setID(uint32_t id) {
        m_id = id;
    }

    uint32_t NetworkMessage::getID() const {
        return m_id;
    }

    void NetworkMessage::setPayload(const std::vector<uint8_t>& data) {
        m_payload = data;
    }

    void NetworkMessage::setPayload(const uint8_t* data, size_t size) {
        m_payload.assign(data, data + size);
    }

    const std::vector<uint8_t>& NetworkMessage::getPayload() const {
        return m_payload;
    }

    uint64_t NetworkMessage::getTimestamp() const {
        return m_timestamp;
    }

    size_t NetworkMessage::getSize() const {
        return NETMESSAGE_HEADER_SIZE + m_payload.size();
    }

    std::vector<uint8_t> NetworkMessage::serialize() const {
        std::vector<uint8_t> buffer;
        buffer.resize(getSize());

        size_t offset = 0;
        buffer[offset++] = static_cast<uint8_t>(m_type);

        std::memcpy(&buffer[offset], &m_id, sizeof(m_id));
        offset += sizeof(m_id);

        uint32_t payloadSize = static_cast<uint32_t>(m_payload.size());
        std::memcpy(&buffer[offset], &payloadSize, sizeof(payloadSize));
        offset += sizeof(payloadSize);

        std::memcpy(&buffer[offset], &m_timestamp, sizeof(m_timestamp));
        offset += sizeof(m_timestamp);

        if (!m_payload.empty()) {
            std::memcpy(&buffer[offset], m_payload.data(), m_payload.size());
        }

        return buffer;
    }

    bool NetworkMessage::deserialize(const std::vector<uint8_t>& buffer) {
        if (buffer.size() < NETMESSAGE_HEADER_SIZE) {
            return false;
        }

        size_t offset = 0;
        m_type = static_cast<MessageType>(buffer[offset++]);

        std::memcpy(&m_id, &buffer[offset], sizeof(m_id));
        offset += sizeof(m_id);

        uint32_t payloadSize = 0;
        std::memcpy(&payloadSize, &buffer[offset], sizeof(payloadSize));
        offset += sizeof(payloadSize);

        std::memcpy(&m_timestamp, &buffer[offset], sizeof(m_timestamp));
        offset += sizeof(m_timestamp);

        if (payloadSize > 0) {
            if (offset + payloadSize > buffer.size()) {
                return false;
            }
            m_payload.assign(&buffer[offset], &buffer[offset] + payloadSize);
        } else {
            m_payload.clear();
        }

        return true;
    }

    NetworkMessage NetworkMessage::createACK(uint32_t ackID) {
        NetworkMessage msg;
        msg.setType(MessageType::Acknowledge);
        msg.setID(ackID);
        msg.m_timestamp = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );
        return msg;
    }

    NetworkMessage NetworkMessage::createPing() {
        NetworkMessage msg;
        msg.setType(MessageType::Ping);
        msg.m_timestamp = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );
        return msg;
    }

    NetworkMessage NetworkMessage::createPong() {
        NetworkMessage msg;
        msg.setType(MessageType::Pong);
        msg.m_timestamp = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );
        return msg;
    }

    void NetworkMessage::reset() {
        m_type = MessageType::Data;
        m_id = 0;
        m_payload.clear();
        m_timestamp = 0;
    }

    MessagePool::MessagePool(size_t initialSize) {
        m_pool.resize(initialSize);
        m_freeList.reserve(initialSize);
        for (size_t i = 0; i < initialSize; ++i) {
            m_freeList.push_back(&m_pool[i]);
        }
    }

    NetworkMessage* MessagePool::acquire() {
        if (m_freeList.empty()) {
            size_t oldSize = m_pool.size();
            size_t newSize = oldSize + (oldSize / 2) + 1;
            m_pool.resize(newSize);
            for (size_t i = oldSize; i < newSize; ++i) {
                m_freeList.push_back(&m_pool[i]);
            }
        }

        NetworkMessage* msg = m_freeList.back();
        m_freeList.pop_back();
        msg->reset();
        ++m_activeCount;
        return msg;
    }

    void MessagePool::release(NetworkMessage* msg) {
        if (!msg) return;
        msg->reset();
        m_freeList.push_back(msg);
        --m_activeCount;
    }

    void MessagePool::clear() {
        m_freeList.clear();
        m_activeCount = 0;
        for (size_t i = 0; i < m_pool.size(); ++i) {
            m_freeList.push_back(&m_pool[i]);
        }
    }

}
