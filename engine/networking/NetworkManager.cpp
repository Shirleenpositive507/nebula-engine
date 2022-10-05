#include "NetworkManager.h"
#include <chrono>
#include <cstring>
#include <algorithm>

namespace nebula {

    NetworkManager& NetworkManager::getInstance() {
        static NetworkManager instance;
        return instance;
    }

    NetworkManager::NetworkManager()
        : m_running(false)
        , m_connected(false)
        , m_port(0)
        , m_nextMessageID(1)
        , m_lastHeartbeatTime(0)
        , m_heartbeatInterval(DEFAULT_HEARTBEAT_INTERVAL_MS)
        , m_timeoutInterval(DEFAULT_TIMEOUT_INTERVAL_MS)
        , m_threadsRunning(false)
        , m_lastStatsReset(0)
        , m_statsIntervalStart(0)
        , m_bytesSentThisInterval(0)
        , m_bytesReceivedThisInterval(0)
    {
    }

    NetworkManager::~NetworkManager() {
        stop();
    }

    bool NetworkManager::start(uint16_t port) {
        if (m_running) return false;

        m_port = port;
        m_running = true;
        m_connected = false;
        m_nextMessageID = 1;
        m_lastStatsReset = getCurrentTimeMs();
        m_statsIntervalStart = m_lastStatsReset;
        m_bytesSentThisInterval = 0;
        m_bytesReceivedThisInterval = 0;

        m_udpSocket.bind(port);
        m_listener.listen(port);

        m_threadsRunning = true;
        m_heartbeatThread = std::make_unique<std::thread>(&NetworkManager::heartbeatLoop, this);
        m_processThread = std::make_unique<std::thread>(&NetworkManager::processOutgoing, this);

        return true;
    }

    void NetworkManager::stop() {
        m_running = false;
        m_connected = false;

        {
            std::lock_guard<std::mutex> lock(m_threadMutex);
            m_threadsRunning = false;
        }

        if (m_heartbeatThread && m_heartbeatThread->joinable()) {
            m_heartbeatThread->join();
        }
        if (m_processThread && m_processThread->joinable()) {
            m_processThread->join();
        }

        m_listener.close();
        m_udpSocket.unbind();
        m_tcpSocket.disconnect();

        {
            std::lock_guard<std::mutex> lock(m_outgoingMutex);
            while (!m_outgoingQueue.empty()) m_outgoingQueue.pop();
        }
        {
            std::lock_guard<std::mutex> lock(m_incomingMutex);
            while (!m_incomingQueue.empty()) m_incomingQueue.pop();
        }
        {
            std::lock_guard<std::mutex> lock(m_ackMutex);
            m_pendingACKs.clear();
            m_lastSendTime.clear();
            m_retryCount.clear();
        }
    }

    bool NetworkManager::isRunning() const {
        return m_running;
    }

    bool NetworkManager::connect(const std::string& host, uint16_t port) {
        if (!m_running) return false;

        bool success = m_tcpSocket.connect(host, port, sf::seconds(5));
        m_connected = success;

        if (success) {
            NetworkMessage connectMsg;
            connectMsg.setType(MessageType::Connect);
            connectMsg.setID(getNextMessageID());
            send(connectMsg);
        }

        return success;
    }

    void NetworkManager::disconnect() {
        if (m_connected) {
            NetworkMessage disconnectMsg;
            disconnectMsg.setType(MessageType::Disconnect);
            disconnectMsg.setID(getNextMessageID());
            send(disconnectMsg);
            m_tcpSocket.disconnect();
            m_connected = false;
        }
    }

    bool NetworkManager::send(NetworkMessage& message) {
        if (!m_running || !m_connected) return false;

        if (message.getID() == 0) {
            message.setID(getNextMessageID());
        }

        {
            std::lock_guard<std::mutex> lock(m_ackMutex);
            if (message.getType() == MessageType::Data) {
                m_pendingACKs[message.getID()] = message;
                m_lastSendTime[message.getID()] = getCurrentTimeMs();
                m_retryCount[message.getID()] = 0;
            }
        }

        Packet packet;
        auto serialized = message.serialize();
        packet.writeInt32(static_cast<int32_t>(serialized.size()));
        for (auto byte : serialized) {
            packet.writeInt8(static_cast<int8_t>(byte));
        }

        bool sent = m_tcpSocket.send(packet);

        if (sent) {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.bytesSent += serialized.size() + 4;
            m_stats.packetsSent++;
            m_stats.messagesSent++;
            m_bytesSentThisInterval += serialized.size() + 4;
        }

        return sent;
    }

    bool NetworkManager::broadcast(NetworkMessage& message) {
        if (!m_running) return false;

        Packet packet;
        auto serialized = message.serialize();
        packet.writeInt32(static_cast<int32_t>(serialized.size()));
        for (auto byte : serialized) {
            packet.writeInt8(static_cast<int8_t>(byte));
        }

        bool sent = m_udpSocket.sendPacket(packet, sf::IpAddress::Broadcast.toString(), m_port);

        if (sent) {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.bytesSent += serialized.size() + 4;
            m_stats.packetsSent++;
            m_stats.messagesSent++;
        }

        return sent;
    }

    void NetworkManager::setPacketHandler(PacketHandler handler) {
        m_handler = handler;
    }

    NetworkStats NetworkManager::getStats() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_stats;
    }

    void NetworkManager::resetStats() {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats = NetworkStats();
        m_lastStatsReset = getCurrentTimeMs();
        m_statsIntervalStart = m_lastStatsReset;
        m_bytesSentThisInterval = 0;
        m_bytesReceivedThisInterval = 0;
    }

    uint32_t NetworkManager::getNextMessageID() {
        return m_nextMessageID++;
    }

    void NetworkManager::update() {
        processIncoming();

        uint64_t now = getCurrentTimeMs();
        if (now - m_statsIntervalStart >= 1000) {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.bytesSentPerSecond = m_bytesSentThisInterval;
            m_stats.bytesReceivedPerSecond = m_bytesReceivedThisInterval;
            m_bytesSentThisInterval = 0;
            m_bytesReceivedThisInterval = 0;
            m_statsIntervalStart = now;
        }

        checkTimeouts();
        resendUnacknowledged();
    }

    void NetworkManager::heartbeatLoop() {
        while (m_threadsRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (!m_running || !m_connected) continue;

            uint64_t now = getCurrentTimeMs();
            if (now - m_lastHeartbeatTime >= m_heartbeatInterval) {
                sendHeartbeat();
                m_lastHeartbeatTime = now;
            }
        }
    }

    void NetworkManager::processOutgoing() {
        while (m_threadsRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            if (!m_running || !m_connected) continue;

            std::queue<NetworkMessage> localQueue;
            {
                std::lock_guard<std::mutex> lock(m_outgoingMutex);
                localQueue.swap(m_outgoingQueue);
            }

            while (!localQueue.empty()) {
                NetworkMessage& msg = localQueue.front();
                send(msg);
                localQueue.pop();
            }
        }
    }

    void NetworkManager::processIncoming() {
        if (!m_running) return;

        Packet packet;
        std::string address;
        uint16_t port;

        while (m_udpSocket.receivePacket(packet, address, port)) {
            int32_t size = packet.readInt32();
            std::vector<uint8_t> buffer;
            buffer.reserve(size);
            for (int32_t i = 0; i < size; ++i) {
                buffer.push_back(static_cast<uint8_t>(packet.readInt8()));
            }

            NetworkMessage msg;
            if (msg.deserialize(buffer)) {
                {
                    std::lock_guard<std::mutex> lock(m_statsMutex);
                    m_stats.bytesReceived += size + 4;
                    m_stats.packetsReceived++;
                    m_stats.messagesReceived++;
                    m_bytesReceivedThisInterval += size + 4;
                }

                if (m_handler) {
                    m_handler(msg);
                }
            }
        }

        Packet tcpPacket;
        while (m_tcpSocket.receive(tcpPacket)) {
            int32_t size = tcpPacket.readInt32();
            std::vector<uint8_t> buffer;
            buffer.reserve(size);
            for (int32_t i = 0; i < size; ++i) {
                buffer.push_back(static_cast<uint8_t>(tcpPacket.readInt8()));
            }

            NetworkMessage msg;
            if (msg.deserialize(buffer)) {
                if (msg.getType() == MessageType::Acknowledge) {
                    std::lock_guard<std::mutex> lock(m_ackMutex);
                    m_pendingACKs.erase(msg.getID());
                    m_lastSendTime.erase(msg.getID());
                    m_retryCount.erase(msg.getID());
                } else if (msg.getType() == MessageType::Ping) {
                    NetworkMessage pong = NetworkMessage::createPong();
                    pong.setID(msg.getID());
                    send(pong);
                } else {
                    NetworkMessage ack = NetworkMessage::createACK(msg.getID());
                    send(ack);

                    {
                        std::lock_guard<std::mutex> lock(m_statsMutex);
                        m_stats.bytesReceived += size + 4;
                        m_stats.packetsReceived++;
                        m_stats.messagesReceived++;
                        m_bytesReceivedThisInterval += size + 4;
                    }

                    if (m_handler) {
                        m_handler(msg);
                    }
                }
            }
        }
    }

    void NetworkManager::sendHeartbeat() {
        NetworkMessage ping = NetworkMessage::createPing();
        ping.setID(getNextMessageID());
        send(ping);
    }

    void NetworkManager::checkTimeouts() {
        if (!m_connected) return;

        uint64_t now = getCurrentTimeMs();
        std::lock_guard<std::mutex> lock(m_ackMutex);

        for (auto& entry : m_lastSendTime) {
            if (now - entry.second >= m_timeoutInterval) {
                m_connected = false;
                m_tcpSocket.disconnect();

                if (m_handler) {
                    NetworkMessage timeoutMsg;
                    timeoutMsg.setType(MessageType::Disconnect);
                    timeoutMsg.setID(entry.first);
                    m_handler(timeoutMsg);
                }
                break;
            }
        }
    }

    void NetworkManager::resendUnacknowledged() {
        if (!m_connected) return;

        uint64_t now = getCurrentTimeMs();
        std::lock_guard<std::mutex> lock(m_ackMutex);

        std::vector<uint32_t> toRemove;
        for (auto& entry : m_pendingACKs) {
            uint32_t id = entry.first;
            if (now - m_lastSendTime[id] >= RESEND_INTERVAL_MS) {
                if (m_retryCount[id] >= MAX_RETRIES) {
                    toRemove.push_back(id);
                    continue;
                }

                Packet packet;
                auto serialized = entry.second.serialize();
                packet.writeInt32(static_cast<int32_t>(serialized.size()));
                for (auto byte : serialized) {
                    packet.writeInt8(static_cast<int8_t>(byte));
                }

                if (m_tcpSocket.send(packet)) {
                    m_retryCount[id]++;
                    m_lastSendTime[id] = now;
                }
            }
        }

        for (auto id : toRemove) {
            m_pendingACKs.erase(id);
            m_lastSendTime.erase(id);
            m_retryCount.erase(id);
        }
    }

    uint64_t NetworkManager::getCurrentTimeMs() {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );
    }

}
