#pragma once

#include "NetworkMessage.h"
#include "TCPSocket.h"
#include "UDPSocket.h"
#include "Packet.h"

#include <string>
#include <cstdint>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <atomic>

namespace nebula {

    struct ConnectionInfo {
        uint32_t id;
        std::string address;
        uint16_t port;
        uint64_t lastHeartbeat;
        uint64_t connectTime;
        bool connected;
    };

    struct NetworkStats {
        uint64_t bytesSent = 0;
        uint64_t bytesReceived = 0;
        uint64_t packetsSent = 0;
        uint64_t packetsReceived = 0;
        uint64_t messagesSent = 0;
        uint64_t messagesReceived = 0;
        uint64_t bytesSentPerSecond = 0;
        uint64_t bytesReceivedPerSecond = 0;
        uint32_t pingMs = 0;
        uint32_t packetLoss = 0;
    };

    class NetworkManager {
    public:
        static NetworkManager& getInstance();

        bool start(uint16_t port);
        void stop();
        bool isRunning() const;

        bool connect(const std::string& host, uint16_t port);
        void disconnect();

        bool send(NetworkMessage& message);
        bool broadcast(NetworkMessage& message);

        using PacketHandler = std::function<void(const NetworkMessage&)>;
        void setPacketHandler(PacketHandler handler);

        NetworkStats getStats() const;
        void resetStats();

        uint32_t getNextMessageID();

        void update();

    private:
        NetworkManager();
        ~NetworkManager();
        NetworkManager(const NetworkManager&) = delete;
        NetworkManager& operator=(const NetworkManager&) = delete;

        void heartbeatLoop();
        void processOutgoing();
        void processIncoming();

        void sendHeartbeat();
        void checkTimeouts();
        void resendUnacknowledged();

        TCPSocket m_tcpSocket;
        TCPListener m_listener;
        UDPSocket m_udpSocket;

        PacketHandler m_handler;

        std::atomic<bool> m_running;
        std::atomic<bool> m_connected;
        uint16_t m_port;

        uint32_t m_nextMessageID;
        uint64_t m_lastHeartbeatTime;
        uint64_t m_heartbeatInterval;
        uint64_t m_timeoutInterval;

        std::queue<NetworkMessage> m_outgoingQueue;
        std::queue<NetworkMessage> m_incomingQueue;
        mutable std::mutex m_outgoingMutex;
        mutable std::mutex m_incomingMutex;

        std::unordered_map<uint32_t, NetworkMessage> m_pendingACKs;
        std::unordered_map<uint32_t, uint64_t> m_lastSendTime;
        std::unordered_map<uint32_t, uint32_t> m_retryCount;
        mutable std::mutex m_ackMutex;

        static constexpr uint32_t MAX_RETRIES = 5;
        static constexpr uint64_t RESEND_INTERVAL_MS = 500;
        static constexpr uint64_t DEFAULT_HEARTBEAT_INTERVAL_MS = 3000;
        static constexpr uint64_t DEFAULT_TIMEOUT_INTERVAL_MS = 10000;

        std::unique_ptr<std::thread> m_heartbeatThread;
        std::unique_ptr<std::thread> m_processThread;
        std::mutex m_threadMutex;
        bool m_threadsRunning;

        NetworkStats m_stats;
        mutable std::mutex m_statsMutex;

        uint64_t m_lastStatsReset;
        uint64_t m_statsIntervalStart;
        uint64_t m_bytesSentThisInterval;
        uint64_t m_bytesReceivedThisInterval;

        static uint64_t getCurrentTimeMs();
    };

}
