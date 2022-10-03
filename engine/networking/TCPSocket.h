#pragma once

#include <SFML/Network/TcpSocket.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <string>
#include <functional>
#include <memory>

namespace nebula {

    class Packet;

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

        sf::TcpSocket& getHandle() { return m_socket; }
        const sf::TcpSocket& getHandle() const { return m_socket; }

        using ConnectCallback = std::function<void(bool success)>;
        using DisconnectCallback = std::function<void()>;

        void connectAsync(const std::string& host, uint16_t port,
                          ConnectCallback callback,
                          sf::Time timeout = sf::seconds(5));
        void disconnectAsync(DisconnectCallback callback);

    private:
        sf::TcpSocket m_socket;
        bool m_connected = false;
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
