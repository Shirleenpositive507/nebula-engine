#pragma once

#include <SFML/Network/UdpSocket.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include <functional>
#include <unordered_map>
#include <deque>
#include <mutex>

namespace nebula {

    class Packet;

    enum class UDPChannelType {
        Unreliable,
        Reliable,
        UnreliableOrdered,
        ReliableOrdered
    };

    struct UDPChannel {
        uint8_t id;
        UDPChannelType type;
        uint16_t sequenceNumber;
        uint16_t lastReceivedSeq;
        std::deque<std::vector<uint8_t>> pendingAcks;
        std::unordered_map<uint16_t, std::vector<uint8_t>> receiveBuffer;
        uint64_t lastSendTime;
    };

    struct BoundAddress {
        sf::IpAddress address;
        uint16_t port;
        bool active;
    };

    class UDPSocket {
    public:
        UDPSocket();
        ~UDPSocket();

        UDPSocket(const UDPSocket&) = delete;
        UDPSocket& operator=(const UDPSocket&) = delete;

        UDPSocket(UDPSocket&& other) noexcept;
        UDPSocket& operator=(UDPSocket&& other) noexcept;

        bool bind(uint16_t port);
        void unbind();

        bool bindToAddress(const std::string& address, uint16_t port);
        bool addBindAddress(const std::string& address);
        bool removeBindAddress(const std::string& address);
        std::vector<BoundAddress> getBoundAddresses() const;

        size_t send(const void* data, size_t size, const std::string& address, uint16_t port);
        size_t receive(void* data, size_t size, std::string& address, uint16_t& port);

        bool sendPacket(Packet& packet, const std::string& address, uint16_t port);
        bool receivePacket(Packet& packet, std::string& address, uint16_t& port);

        void setBlocking(bool blocking);
        bool isBlocking() const;

        void setTimeout(sf::Time timeout);

        bool joinMulticast(const std::string& multicastAddress);
        bool leaveMulticast(const std::string& multicastAddress);

        size_t sendBroadcast(const void* data, size_t size, uint16_t port);
        bool sendBroadcastPacket(Packet& packet, uint16_t port);

        uint16_t getLocalPort() const;

        uint8_t createChannel(UDPChannelType type);
        bool removeChannel(uint8_t channelId);
        UDPChannel* getChannel(uint8_t channelId);

        bool sendOnChannel(uint8_t channelId, const void* data, size_t size,
                           const std::string& address, uint16_t port);
        bool receiveOnChannel(uint8_t channelId, void* data, size_t& size,
                              std::string& address, uint16_t& port);

        size_t getChannelCount() const;

        void setAggregationEnabled(bool enable);
        bool isAggregationEnabled() const;
        void setAggregationThreshold(size_t bytes);
        size_t getAggregationThreshold() const;
        void flushAggregatedPackets(const std::string& address, uint16_t port);

        sf::UdpSocket& getHandle() { return m_socket; }
        const sf::UdpSocket& getHandle() const { return m_socket; }

    private:
        void sendAggregated(const std::string& address, uint16_t port);

        sf::UdpSocket m_socket;
        bool m_bound = false;

        std::vector<sf::UdpSocket> m_multiHomeSockets;
        std::vector<BoundAddress> m_boundAddresses;
        mutable std::mutex m_bindMutex;

        std::unordered_map<uint8_t, UDPChannel> m_channels;
        uint8_t m_nextChannelId;
        mutable std::mutex m_channelMutex;

        bool m_aggregationEnabled;
        size_t m_aggregationThreshold;
        std::vector<uint8_t> m_aggregationBuffer;
        std::string m_aggregationAddress;
        uint16_t m_aggregationPort;
        mutable std::mutex m_aggMutex;
    };

}

}
