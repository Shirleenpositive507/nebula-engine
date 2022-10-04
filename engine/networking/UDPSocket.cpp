#include "UDPSocket.h"
#include "Packet.h"
#include <cstring>

namespace nebula {

    UDPSocket::UDPSocket() {
        m_socket.setBlocking(true);
    }

    UDPSocket::~UDPSocket() {
        unbind();
    }

    UDPSocket::UDPSocket(UDPSocket&& other) noexcept
        : m_socket(std::move(other.m_socket))
        , m_bound(other.m_bound)
    {
        other.m_bound = false;
    }

    UDPSocket& UDPSocket::operator=(UDPSocket&& other) noexcept {
        if (this != &other) {
            unbind();
            m_socket = std::move(other.m_socket);
            m_bound = other.m_bound;
            other.m_bound = false;
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
    }

    size_t UDPSocket::send(const void* data, size_t size, const std::string& address, uint16_t port) {
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

}
