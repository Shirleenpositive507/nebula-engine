#include "UDPSocket.h"
#include "Packet.h"
#include <cstring>
#include <algorithm>

namespace nebula {

    UDPSocket::UDPSocket()
        : m_nextChannelId(0)
        , m_aggregationEnabled(false)
        , m_aggregationThreshold(512)
        , m_aggregationPort(0)
    {
        m_socket.setBlocking(true);
    }

    UDPSocket::~UDPSocket() {
        unbind();
    }

    UDPSocket::UDPSocket(UDPSocket&& other) noexcept
        : m_socket(std::move(other.m_socket))
        , m_bound(other.m_bound)
        , m_multiHomeSockets(std::move(other.m_multiHomeSockets))
        , m_boundAddresses(std::move(other.m_boundAddresses))
        , m_channels(std::move(other.m_channels))
        , m_nextChannelId(other.m_nextChannelId)
        , m_aggregationEnabled(other.m_aggregationEnabled)
        , m_aggregationThreshold(other.m_aggregationThreshold)
    {
        other.m_bound = false;
        other.m_nextChannelId = 0;
    }

    UDPSocket& UDPSocket::operator=(UDPSocket&& other) noexcept {
        if (this != &other) {
            unbind();
            m_socket = std::move(other.m_socket);
            m_bound = other.m_bound;
            m_multiHomeSockets = std::move(other.m_multiHomeSockets);
            m_boundAddresses = std::move(other.m_boundAddresses);
            m_channels = std::move(other.m_channels);
            m_nextChannelId = other.m_nextChannelId;
            m_aggregationEnabled = other.m_aggregationEnabled;
            m_aggregationThreshold = other.m_aggregationThreshold;
            other.m_bound = false;
            other.m_nextChannelId = 0;
        }
        return *this;
    }

    bool UDPSocket::bind(uint16_t port) {
        sf::Socket::Status status = m_socket.bind(port);
        m_bound = (status == sf::Socket::Done);
        return m_bound;
    }

    void UDPSocket::unbind() {
        if (m_bound) {
            m_socket.unbind();
            m_bound = false;
        }
        {
            std::lock_guard<std::mutex> lock(m_bindMutex);
            for (auto& sock : m_multiHomeSockets) {
                sock.unbind();
            }
            m_multiHomeSockets.clear();
            m_boundAddresses.clear();
        }
    }

    bool UDPSocket::bindToAddress(const std::string& address, uint16_t port) {
        sf::IpAddress addr(address);
        sf::UdpSocket sock;
        sf::Socket::Status status = sock.bind(port);
        if (status != sf::Socket::Done) return false;

        std::lock_guard<std::mutex> lock(m_bindMutex);
        m_multiHomeSockets.push_back(std::move(sock));
        m_boundAddresses.push_back({addr, port, true});
        return true;
    }

    bool UDPSocket::addBindAddress(const std::string& address) {
        std::lock_guard<std::mutex> lock(m_bindMutex);
        for (auto& ba : m_boundAddresses) {
            if (ba.address.toString() == address) {
                ba.active = true;
                return true;
            }
        }
        return false;
    }

    bool UDPSocket::removeBindAddress(const std::string& address) {
        std::lock_guard<std::mutex> lock(m_bindMutex);
        for (auto& ba : m_boundAddresses) {
            if (ba.address.toString() == address) {
                ba.active = false;
                return true;
            }
        }
        return false;
    }

    std::vector<BoundAddress> UDPSocket::getBoundAddresses() const {
        std::lock_guard<std::mutex> lock(m_bindMutex);
        return m_boundAddresses;
    }

    size_t UDPSocket::send(const void* data, size_t size, const std::string& address, uint16_t port) {
        if (m_aggregationEnabled) {
            std::lock_guard<std::mutex> lock(m_aggMutex);
            if (m_aggregationBuffer.empty()) {
                m_aggregationAddress = address;
                m_aggregationPort = port;
            }
            m_aggregationBuffer.insert(m_aggregationBuffer.end(),
                static_cast<const uint8_t*>(data),
                static_cast<const uint8_t*>(data) + size);
            if (m_aggregationBuffer.size() >= m_aggregationThreshold) {
                sendAggregated(m_aggregationAddress, m_aggregationPort);
            }
            return size;
        }

        sf::IpAddress addr(address);
        sf::Socket::Status status = m_socket.send(data, size, addr, port);
        return (status == sf::Socket::Done) ? size : 0;
    }

    size_t UDPSocket::receive(void* data, size_t size, std::string& address, uint16_t& port) {
        sf::IpAddress addr;
        std::size_t received = 0;
        sf::Socket::Status status = m_socket.receive(data, size, received, addr, port);
        if (status == sf::Socket::Done) {
            address = addr.toString();
            return received;
        }
        return 0;
    }

    bool UDPSocket::sendPacket(Packet& packet, const std::string& address, uint16_t port) {
        sf::IpAddress addr(address);
        return m_socket.send(packet.getHandle(), addr, port) == sf::Socket::Done;
    }

    bool UDPSocket::receivePacket(Packet& packet, std::string& address, uint16_t& port) {
        sf::IpAddress addr;
        sf::Socket::Status status = m_socket.receive(packet.getHandle(), addr, port);
        if (status == sf::Socket::Done) {
            address = addr.toString();
            return true;
        }
        return false;
    }

    void UDPSocket::setBlocking(bool blocking) {
        m_socket.setBlocking(blocking);
        for (auto& sock : m_multiHomeSockets) {
            sock.setBlocking(blocking);
        }
    }

    bool UDPSocket::isBlocking() const {
        return m_socket.isBlocking();
    }

    void UDPSocket::setTimeout(sf::Time timeout) {
        (void)timeout;
    }

    bool UDPSocket::joinMulticast(const std::string& multicastAddress) {
        sf::IpAddress addr(multicastAddress);
        return m_socket.joinMulticastGroup(addr) == sf::Socket::Done;
    }

    bool UDPSocket::leaveMulticast(const std::string& multicastAddress) {
        sf::IpAddress addr(multicastAddress);
        return m_socket.leaveMulticastGroup(addr) == sf::Socket::Done;
    }

    size_t UDPSocket::sendBroadcast(const void* data, size_t size, uint16_t port) {
        return send(data, size, sf::IpAddress::Broadcast.toString(), port);
    }

    bool UDPSocket::sendBroadcastPacket(Packet& packet, uint16_t port) {
        return sendPacket(packet, sf::IpAddress::Broadcast.toString(), port);
    }

    uint16_t UDPSocket::getLocalPort() const {
        return m_socket.getLocalPort();
    }

    uint8_t UDPSocket::createChannel(UDPChannelType type) {
        std::lock_guard<std::mutex> lock(m_channelMutex);
        uint8_t id = m_nextChannelId++;
        UDPChannel channel;
        channel.id = id;
        channel.type = type;
        channel.sequenceNumber = 0;
        channel.lastReceivedSeq = 0;
        channel.lastSendTime = 0;
        m_channels[id] = channel;
        return id;
    }

    bool UDPSocket::removeChannel(uint8_t channelId) {
        std::lock_guard<std::mutex> lock(m_channelMutex);
        return m_channels.erase(channelId) > 0;
    }

    UDPChannel* UDPSocket::getChannel(uint8_t channelId) {
        std::lock_guard<std::mutex> lock(m_channelMutex);
        auto it = m_channels.find(channelId);
        return (it != m_channels.end()) ? &it->second : nullptr;
    }

    bool UDPSocket::sendOnChannel(uint8_t channelId, const void* data, size_t size,
                                   const std::string& address, uint16_t port) {
        UDPChannel* channel = getChannel(channelId);
        if (!channel) return false;

        channel->sequenceNumber++;
        channel->lastSendTime = static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );

        size_t totalSize = 3 + size;
        std::vector<uint8_t> packet;
        packet.reserve(totalSize);
        packet.push_back(channelId);
        packet.push_back(static_cast<uint8_t>(channel->sequenceNumber & 0xFF));
        packet.push_back(static_cast<uint8_t>((channel->sequenceNumber >> 8) & 0xFF));
        packet.insert(packet.end(), static_cast<const uint8_t*>(data),
                      static_cast<const uint8_t*>(data) + size);

        if (channel->type == UDPChannelType::Reliable ||
            channel->type == UDPChannelType::ReliableOrdered) {
            channel->pendingAcks.push_back(packet);
        }

        return send(packet.data(), packet.size(), address, port) > 0;
    }

    bool UDPSocket::receiveOnChannel(uint8_t channelId, void* data, size_t& size,
                                      std::string& address, uint16_t& port) {
        UDPChannel* channel = getChannel(channelId);
        if (!channel) return false;

        uint8_t recvBuf[2048];
        size_t received = receive(recvBuf, sizeof(recvBuf), address, port);
        if (received < 3) return false;

        uint8_t recvChannel = recvBuf[0];
        if (recvChannel != channelId) return false;

        uint16_t seqNum = static_cast<uint16_t>(recvBuf[1]) |
                          (static_cast<uint16_t>(recvBuf[2]) << 8);

        if (channel->type == UDPChannelType::ReliableOrdered ||
            channel->type == UDPChannelType::UnreliableOrdered) {
            if (seqNum <= channel->lastReceivedSeq) return false;
            channel->lastReceivedSeq = seqNum;
        }

        size_t payloadSize = received - 3;
        if (data && payloadSize <= size) {
            std::memcpy(data, recvBuf + 3, payloadSize);
            size = payloadSize;
            return true;
        }
        size = payloadSize;
        return false;
    }

    size_t UDPSocket::getChannelCount() const {
        std::lock_guard<std::mutex> lock(m_channelMutex);
        return m_channels.size();
    }

    void UDPSocket::setAggregationEnabled(bool enable) {
        m_aggregationEnabled = enable;
        if (!enable) {
            std::lock_guard<std::mutex> lock(m_aggMutex);
            if (!m_aggregationBuffer.empty()) {
                sendAggregated(m_aggregationAddress, m_aggregationPort);
            }
        }
    }

    bool UDPSocket::isAggregationEnabled() const {
        return m_aggregationEnabled;
    }

    void UDPSocket::setAggregationThreshold(size_t bytes) {
        m_aggregationThreshold = bytes;
    }

    size_t UDPSocket::getAggregationThreshold() const {
        return m_aggregationThreshold;
    }

    void UDPSocket::flushAggregatedPackets(const std::string& address, uint16_t port) {
        if (m_aggregationEnabled) {
            std::lock_guard<std::mutex> lock(m_aggMutex);
            sendAggregated(address, port);
        }
    }

    void UDPSocket::sendAggregated(const std::string& address, uint16_t port) {
        if (m_aggregationBuffer.empty()) return;
        sf::IpAddress addr(address);
        m_socket.send(m_aggregationBuffer.data(), m_aggregationBuffer.size(), addr, port);
        m_aggregationBuffer.clear();
    }

}
