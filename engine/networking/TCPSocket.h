#pragma once

#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/SocketSelector.hpp>
#include <string>
#include <functional>
#include <memory>
#include <atomic>

namespace nebula {

    class Packet;

    enum class SocketOption {
        TCPNoDelay,
        KeepAlive,
        SendBufferSize,
        RecvBufferSize,
        ReuseAddress,
        NonBlocking
    };

    class TCPSocket {
    public:
        TCPSocket();
        ~TCPSocket();

        TCPSocket(const TCPSocket&) = delete;
        TCPSocket& operator=(const TCPSocket&) = delete;

        TCPSocket(TCPSocket&& other) noexcept;
        TCPSocket& operator=(TCPSocket&& other) noexcept;

        bool connect(const std::string& host, uint16_t port, sf::Time timeout = sf::Time::Zero);
        void disconnect();

        bool send(Packet& packet);
        bool receive(Packet& packet);

        bool isConnected() const;

        std::string getRemoteAddress() const;
        uint16_t getRemotePort() const;

        void setBlocking(bool blocking);
        bool isBlocking() const;

        void setTimeout(sf::Time timeout);

        void setOption(SocketOption option, int value);
        int getOption(SocketOption option) const;

        void setTcpNoDelay(bool enable);
        bool getTcpNoDelay() const;

        void setKeepAlive(bool enable);
        bool getKeepAlive() const;

        void setSendBufferSize(int size);
        int getSendBufferSize() const;

        void setRecvBufferSize(int size);
        int getRecvBufferSize() const;

        void setConnectionTimeout(sf::Time timeout);
        sf::Time getConnectionTimeout() const;

        bool connectNonBlocking(const std::string& host, uint16_t port);
        bool isNonBlockingConnectComplete() const;

        sf::TcpSocket& getHandle() { return m_socket; }
        const sf::TcpSocket& getHandle() const { return m_socket; }

        using ConnectCallback = std::function<void(bool success)>;
        using DisconnectCallback = std::function<void()>;

        void connectAsync(const std::string& host, uint16_t port,
                          ConnectCallback callback,
                          sf::Time timeout = sf::seconds(5));
        void disconnectAsync(DisconnectCallback callback);

        bool startTLS();
        bool isTLSEnabled() const;

    private:
        bool applySocketOption(int level, int optname, int value);

        sf::TcpSocket m_socket;
        bool m_connected = false;
        bool m_tlsEnabled = false;
        bool m_tcpNoDelay = false;
        bool m_keepAlive = false;
        int m_sendBufferSize = 0;
        int m_recvBufferSize = 0;
        sf::Time m_connectionTimeout = sf::seconds(5);
        std::atomic<bool> m_nonBlockingConnectInProgress{false};
    };

    class TCPListener {
    public:
        TCPListener();
        ~TCPListener();

        TCPListener(const TCPListener&) = delete;
        TCPListener& operator=(const TCPListener&) = delete;

        TCPListener(TCPListener&& other) noexcept;
        TCPListener& operator=(TCPListener&& other) noexcept;

        bool listen(uint16_t port);
        bool accept(TCPSocket& socket);
        void close();
        bool isListening() const;

        uint16_t getLocalPort() const;

        sf::TcpListener& getHandle() { return m_listener; }
        const sf::TcpListener& getHandle() const { return m_listener; }

    private:
        sf::TcpListener m_listener;
        bool m_listening = false;
    };

}
