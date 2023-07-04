#include "TCPSocket.h"
#include "Packet.h"
#include <thread>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#endif

namespace nebula {

    TCPSocket::TCPSocket()
        : m_tlsEnabled(false)
    {
        m_socket.setBlocking(true);
    }

    TCPSocket::~TCPSocket() {
        disconnect();
    }

    TCPSocket::TCPSocket(TCPSocket&& other) noexcept
        : m_socket(std::move(other.m_socket))
        , m_connected(other.m_connected)
        , m_tlsEnabled(other.m_tlsEnabled)
        , m_tcpNoDelay(other.m_tcpNoDelay)
        , m_keepAlive(other.m_keepAlive)
        , m_sendBufferSize(other.m_sendBufferSize)
        , m_recvBufferSize(other.m_recvBufferSize)
        , m_connectionTimeout(other.m_connectionTimeout)
    {
        other.m_connected = false;
        other.m_tlsEnabled = false;
    }

    TCPSocket& TCPSocket::operator=(TCPSocket&& other) noexcept {
        if (this != &other) {
            disconnect();
            m_socket = std::move(other.m_socket);
            m_connected = other.m_connected;
            m_tlsEnabled = other.m_tlsEnabled;
            m_tcpNoDelay = other.m_tcpNoDelay;
            m_keepAlive = other.m_keepAlive;
            m_sendBufferSize = other.m_sendBufferSize;
            m_recvBufferSize = other.m_recvBufferSize;
            m_connectionTimeout = other.m_connectionTimeout;
            other.m_connected = false;
            other.m_tlsEnabled = false;
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

        if (m_connected) {
            applySocketOption(IPPROTO_TCP, TCP_NODELAY, m_tcpNoDelay ? 1 : 0);

#ifdef _WIN32
            char keepAlive = m_keepAlive ? 1 : 0;
            setsockopt(m_socket.getNativeHandle(), SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive));
#else
            int keepAlive = m_keepAlive ? 1 : 0;
            setsockopt(m_socket.getNativeHandle(), SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive));
#endif

            if (m_sendBufferSize > 0) {
                setsockopt(m_socket.getNativeHandle(), SOL_SOCKET, SO_SNDBUF,
                           reinterpret_cast<const char*>(&m_sendBufferSize), sizeof(m_sendBufferSize));
            }
            if (m_recvBufferSize > 0) {
                setsockopt(m_socket.getNativeHandle(), SOL_SOCKET, SO_RCVBUF,
                           reinterpret_cast<const char*>(&m_recvBufferSize), sizeof(m_recvBufferSize));
            }
        }

        return m_connected;
    }

    void TCPSocket::disconnect() {
        if (m_connected) {
            m_socket.disconnect();
            m_connected = false;
        }
        m_nonBlockingConnectInProgress = false;
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
        m_socket.setBlocking(false);
    }

    void TCPSocket::setOption(SocketOption option, int value) {
        switch (option) {
            case SocketOption::TCPNoDelay:
                setTcpNoDelay(value != 0);
                break;
            case SocketOption::KeepAlive:
                setKeepAlive(value != 0);
                break;
            case SocketOption::SendBufferSize:
                setSendBufferSize(value);
                break;
            case SocketOption::RecvBufferSize:
                setRecvBufferSize(value);
                break;
            case SocketOption::ReuseAddress:
                if (m_connected) {
                    int reuse = value;
                    setsockopt(m_socket.getNativeHandle(), SOL_SOCKET, SO_REUSEADDR,
                               reinterpret_cast<const char*>(&reuse), sizeof(reuse));
                }
                break;
            case SocketOption::NonBlocking:
                setBlocking(value == 0);
                break;
        }
    }

    int TCPSocket::getOption(SocketOption option) const {
        switch (option) {
            case SocketOption::TCPNoDelay: return m_tcpNoDelay ? 1 : 0;
            case SocketOption::KeepAlive: return m_keepAlive ? 1 : 0;
            case SocketOption::SendBufferSize: return m_sendBufferSize;
            case SocketOption::RecvBufferSize: return m_recvBufferSize;
            default: return 0;
        }
    }

    void TCPSocket::setTcpNoDelay(bool enable) {
        m_tcpNoDelay = enable;
        if (m_connected) {
            applySocketOption(IPPROTO_TCP, TCP_NODELAY, enable ? 1 : 0);
        }
    }

    bool TCPSocket::getTcpNoDelay() const {
        return m_tcpNoDelay;
    }

    void TCPSocket::setKeepAlive(bool enable) {
        m_keepAlive = enable;
        if (m_connected) {
#ifdef _WIN32
            char opt = enable ? 1 : 0;
            setsockopt(m_socket.getNativeHandle(), SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
#else
            int opt = enable ? 1 : 0;
            setsockopt(m_socket.getNativeHandle(), SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
#endif
        }
    }

    bool TCPSocket::getKeepAlive() const {
        return m_keepAlive;
    }

    void TCPSocket::setSendBufferSize(int size) {
        m_sendBufferSize = size;
        if (m_connected && size > 0) {
            setsockopt(m_socket.getNativeHandle(), SOL_SOCKET, SO_SNDBUF,
                       reinterpret_cast<const char*>(&size), sizeof(size));
        }
    }

    int TCPSocket::getSendBufferSize() const {
        return m_sendBufferSize;
    }

    void TCPSocket::setRecvBufferSize(int size) {
        m_recvBufferSize = size;
        if (m_connected && size > 0) {
            setsockopt(m_socket.getNativeHandle(), SOL_SOCKET, SO_RCVBUF,
                       reinterpret_cast<const char*>(&size), sizeof(size));
        }
    }

    int TCPSocket::getRecvBufferSize() const {
        return m_recvBufferSize;
    }

    void TCPSocket::setConnectionTimeout(sf::Time timeout) {
        m_connectionTimeout = timeout;
    }

    sf::Time TCPSocket::getConnectionTimeout() const {
        return m_connectionTimeout;
    }

    bool TCPSocket::connectNonBlocking(const std::string& host, uint16_t port) {
        sf::IpAddress address(host);
        m_socket.setBlocking(false);
        sf::Socket::Status status = m_socket.connect(address, port);
        if (status == sf::Socket::Done) {
            m_connected = true;
            m_nonBlockingConnectInProgress = false;
            return true;
        }
        if (status == sf::Socket::NotReady) {
            m_nonBlockingConnectInProgress = true;
            return false;
        }
        m_nonBlockingConnectInProgress = false;
        return false;
    }

    bool TCPSocket::isNonBlockingConnectComplete() const {
        if (!m_nonBlockingConnectInProgress) return false;
        sf::SocketSelector selector;
        selector.add(m_socket);
        if (selector.wait(sf::Time::Zero)) {
            if (selector.isReady(m_socket)) {
                const_cast<TCPSocket*>(this)->m_connected = true;
                const_cast<TCPSocket*>(this)->m_nonBlockingConnectInProgress = false;
                const_cast<TCPSocket*>(this)->m_socket.setBlocking(true);
                return true;
            }
        }
        return false;
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

    bool TCPSocket::startTLS() {
        m_tlsEnabled = true;
        return true;
    }

    bool TCPSocket::isTLSEnabled() const {
        return m_tlsEnabled;
    }

    bool TCPSocket::applySocketOption(int level, int optname, int value) {
#ifdef _WIN32
        char optval = static_cast<char>(value);
        return setsockopt(m_socket.getNativeHandle(), level, optname, &optval, sizeof(optval)) == 0;
#else
        return setsockopt(m_socket.getNativeHandle(), level, optname, &value, sizeof(value)) == 0;
#endif
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
