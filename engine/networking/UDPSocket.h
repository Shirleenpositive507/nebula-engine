#pragma once

#include <SFML/Network/UdpSocket.hpp>
#include <SFML/Network/IpAddress.hpp>
#include <string>
#include <cstdint>
#include <vector>
#include <functional>

namespace nebula {

    class Packet;

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

        sf::UdpSocket& getHandle() { return m_socket; }
        const sf::UdpSocket& getHandle() const { return m_socket; }

    private:
        sf::UdpSocket m_socket;
        bool m_bound = false;
    };

}
