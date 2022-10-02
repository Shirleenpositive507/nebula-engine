#include "Packet.h"

namespace nebula {

    Packet::Packet() = default;

    void Packet::clear() {
        m_packet.clear();
    }

    bool Packet::endOfPacket() const {
        return static_cast<bool>(m_packet);
    }

    const void* Packet::getData() const {
        return m_packet.getData();
    }

    size_t Packet::getDataSize() const {
        return m_packet.getDataSize();
    }

    void Packet::writeString(const std::string& value) {
        m_packet << value;
    }

    std::string Packet::readString() {
        std::string value;
        m_packet >> value;
        return value;
    }

    void Packet::writeVector2(float x, float y) {
        m_packet << x << y;
    }

    void Packet::readVector2(float& x, float& y) {
        m_packet >> x >> y;
    }

    void Packet::writeVector3(float x, float y, float z) {
        m_packet << x << y << z;
    }

    void Packet::readVector3(float& x, float& y, float& z) {
        m_packet >> x >> y >> z;
    }

    void Packet::writeVector4(float x, float y, float z, float w) {
        m_packet << x << y << z << w;
    }

    void Packet::readVector4(float& x, float& y, float& z, float& w) {
        m_packet >> x >> y >> z >> w;
    }

    void Packet::writeInt8(int8_t value) {
        m_packet << value;
    }

    int8_t Packet::readInt8() {
        int8_t value = 0;
        m_packet >> value;
        return value;
    }

    void Packet::writeInt16(int16_t value) {
        m_packet << value;
    }

    int16_t Packet::readInt16() {
        int16_t value = 0;
        m_packet >> value;
        return value;
    }

    void Packet::writeInt32(int32_t value) {
        m_packet << value;
    }

    int32_t Packet::readInt32() {
        int32_t value = 0;
        m_packet >> value;
        return value;
    }

    void Packet::writeInt64(int64_t value) {
        m_packet << value;
    }

    int64_t Packet::readInt64() {
        int64_t value = 0;
        m_packet >> value;
        return value;
    }

    void Packet::writeFloat(float value) {
        m_packet << value;
    }

    float Packet::readFloat() {
        float value = 0.0f;
        m_packet >> value;
        return value;
    }

    void Packet::writeBool(bool value) {
        m_packet << value;
    }

    bool Packet::readBool() {
        bool value = false;
        m_packet >> value;
        return value;
    }

    size_t Packet::getRemainingBytes() const {
        return m_packet.getDataSize();
    }

    void Packet::reserve(size_t size) {
        (void)size;
    }

    PacketPool::PacketPool(size_t initialSize) {
        m_pool.resize(initialSize);
        m_freeList.reserve(initialSize);
        for (size_t i = 0; i < initialSize; ++i) {
            m_freeList.push_back(&m_pool[i]);
        }
    }

    Packet* PacketPool::acquire() {
        if (m_freeList.empty()) {
            size_t oldSize = m_pool.size();
            size_t newSize = oldSize + (oldSize / 2) + 1;
            m_pool.resize(newSize);
            for (size_t i = oldSize; i < newSize; ++i) {
                m_freeList.push_back(&m_pool[i]);
            }
        }

        Packet* p = m_freeList.back();
        m_freeList.pop_back();
        p->clear();
        ++m_activeCount;
        return p;
    }

    void PacketPool::release(Packet* packet) {
        if (!packet) return;
        packet->clear();
        m_freeList.push_back(packet);
        --m_activeCount;
    }

    void PacketPool::clear() {
        m_freeList.clear();
        m_activeCount = 0;
        for (size_t i = 0; i < m_pool.size(); ++i) {
            m_freeList.push_back(&m_pool[i]);
        }
    }

}
