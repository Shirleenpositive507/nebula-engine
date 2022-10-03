#include "TCPSocket.h"
#include "Packet.h"
#include <thread>

namespace nebula {

    TCPSocket::TCPSocket() {
        m_socket.setBlocking(true);
    }

    TCPSocket::~TCPSocket() {
        disconnect();
    }

    TCPSocket::TCPSocket(TCPSocket&& other) noexcept
        : m_socket(std::move(other.m_socket))
        , m_connected(other.m_connected)
    {
        other.m_connected = false;
    }

    TCPSocket& TCPSocket::operator=(TCPSocket&& other) noexcept {
        if (this != &other) {
            disconnect();
            m_socket = std::move(other.m_socket);
            m_connected = other.m_connected;
            other.m_connected = false;
        }
        return *this;
    }

    bool TCPSocket::connect(const std::string& host, uint16_t port, sf::Time timeout) {
        sf::IpAddress address(host);
        sf::Socket::Status status;

        if (timeout == sf::Time::Zero) {
            status = m_socket.connect(address, port);
        } else {
            status = m_socket.connect(address, port, timeout);
        }

        m_connected = (status == sf::Socket::Done);
        return m_connected;
    }

    void TCPSocket::disconnect() {
        if (m_connected) {
            m_socket.disconnect();
            m_connected = false;
        }
    }

    bool TCPSocket::send(Packet& packet) {
        if (!m_connected) return false;
        return m_socket.send(packet.getHandle()) == sf::Socket::Done;
    }

    bool TCPSocket::receive(Packet& packet) {
        if (!m_connected) return false;
        sf::Socket::Status status = m_socket.receive(packet.getHandle());
        if (status == sf::Socket::Disconnected) {
            m_connected = false;
        }
        return status == sf::Socket::Done;
    }

    bool TCPSocket::isConnected() const {
        return m_connected;
    }

    std::string TCPSocket::getRemoteAddress() const {
        return m_socket.getRemoteAddress().toString();
    }

    uint16_t TCPSocket::getRemotePort() const {
        return m_socket.getRemotePort();
    }

    void TCPSocket::setBlocking(bool blocking) {
        m_socket.setBlocking(blocking);
    }

    bool TCPSocket::isBlocking() const {
        return m_socket.isBlocking();
    }

    void TCPSocket::setTimeout(sf::Time timeout) {
        (void)timeout;
    }

    void TCPSocket::connectAsync(const std::string& host, uint16_t port,
                                 ConnectCallback callback, sf::Time timeout)
    {
        std::thread t([this, host, port, callback, timeout]() {
            bool success = connect(host, port, timeout);
            if (callback) {
                callback(success);
            }
        });
        t.detach();
    }

    void TCPSocket::disconnectAsync(DisconnectCallback callback) {
        std::thread t([this, callback]() {
            disconnect();
            if (callback) {
                callback();
            }
        });
        t.detach();
    }

    TCPListener::TCPListener() = default;

    TCPListener::~TCPListener() {
        close();
    }

    TCPListener::TCPListener(TCPListener&& other) noexcept
        : m_listener(std::move(other.m_listener))
        , m_listening(other.m_listening)
    {
        other.m_listening = false;
    }

    TCPListener& TCPListener::operator=(TCPListener&& other) noexcept {
        if (this != &other) {
            close();
            m_listener = std::move(other.m_listener);
            m_listening = other.m_listening;
            other.m_listening = false;
        }
        return *this;
    }

    bool TCPListener::listen(uint16_t port) {
        sf::Socket::Status status = m_listener.listen(port);
        m_listening = (status == sf::Socket::Done);
        return m_listening;
    }

    bool TCPListener::accept(TCPSocket& socket) {
        if (!m_listening) return false;
        sf::TcpSocket& rawSocket = socket.getHandle();
        sf::Socket::Status status = m_listener.accept(rawSocket);
        if (status == sf::Socket::Done) {
            socket.getHandle().setBlocking(false);
            return true;
        }
        return false;
    }

    void TCPListener::close() {
        if (m_listening) {
            m_listener.close();
            m_listening = false;
        }
    }

    bool TCPListener::isListening() const {
        return m_listening;
    }

    uint16_t TCPListener::getLocalPort() const {
        return m_listener.getLocalPort();
    }

}
