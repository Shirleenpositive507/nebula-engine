#include "NetworkManager.h"
#include <chrono>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <cmath>

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
        , m_autoReconnect(true)
        , m_maxReconnectionAttempts(DEFAULT_MAX_RECONNECT_ATTEMPTS)
        , m_reconnectDelayMs(DEFAULT_RECONNECT_DELAY_MS)
        , m_lastReconnectTime(0)
        , m_reconnectionAttempts(0)
        , m_lastPort(0)
        , m_maxPoolSize(DEFAULT_POOL_SIZE)
        , m_encryptionEnabled(false)
        , m_bandwidthLimitBytesPerSec(0)
        , m_bandwidthUsedThisSecond(0)
        , m_bandwidthIntervalStart(0)
        , m_latencyJitterEnabled(true)
        , m_lastLatencyMs(0.0)
        , m_lastJitterMs(0.0)
    {
        m_latencyHistory.resize(LATENCY_HISTORY_SIZE, 0.0);
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
        m_bandwidthIntervalStart = m_lastStatsReset;
        m_bandwidthUsedThisSecond = 0;
        m_reconnectionAttempts = 0;
        m_lastReconnectTime = 0;

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
        if (m_tcpSocket.getLocalPort() > 0) {
            m_tcpSocket.disconnect();
        }

        {
            std::lock_guard<std::mutex> lock(m_poolMutex);
            for (auto& pc : m_connectionPool) {
                if (pc.socket.getLocalPort() > 0) {
                    pc.socket.disconnect();
                }
            }
            m_connectionPool.clear();
        }

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

        m_reconnectionAttempts = 0;
        m_lastHost = host;
        m_lastPort = port;

        bool success = m_tcpSocket.connect(host, port, sf::seconds(5));
        m_connected = success;

        if (success) {
            NetworkMessage connectMsg;
            connectMsg.setType(MessageType::Connect);
            connectMsg.setID(getNextMessageID());
            send(connectMsg);
            m_lastReconnectTime = getCurrentTimeMs();

            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.activeConnections++;
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
            m_reconnectionAttempts = 0;

            std::lock_guard<std::mutex> lock(m_statsMutex);
            if (m_stats.activeConnections > 0) m_stats.activeConnections--;
        }
    }

    void NetworkManager::disconnectAll() {
        disconnect();
        {
            std::lock_guard<std::mutex> lock(m_poolMutex);
            for (auto& pc : m_connectionPool) {
                pc.socket.disconnect();
                pc.inUse = false;
            }
            m_connectionPool.clear();
        }
    }

    bool NetworkManager::send(NetworkMessage& message) {
        if (!m_running || !m_connected) return false;

        if (m_bandwidthLimitBytesPerSec > 0) {
            uint64_t now = getCurrentTimeMs();
            if (now - m_bandwidthIntervalStart >= 1000) {
                m_bandwidthUsedThisSecond = 0;
                m_bandwidthIntervalStart = now;
            }
        }

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
        if (m_encryptionEnabled) {
            encryptData(serialized);
        }
        packet.writeInt32(static_cast<int32_t>(serialized.size()));
        for (auto byte : serialized) {
            packet.writeInt8(static_cast<int8_t>(byte));
        }

        if (m_bandwidthLimitBytesPerSec > 0) {
            size_t packetSize = serialized.size() + 4;
            if (m_bandwidthUsedThisSecond + packetSize > m_bandwidthLimitBytesPerSec) {
                return false;
            }
            m_bandwidthUsedThisSecond += packetSize;
        }

        bool sent = m_tcpSocket.send(packet);

        if (sent) {
            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.bytesSent += serialized.size() + 4;
            m_stats.packetsSent++;
            m_stats.messagesSent++;
            m_bytesSentThisInterval += serialized.size() + 4;
            if (m_encryptionEnabled) {
                m_stats.totalBytesEncrypted += serialized.size();
            }
        }

        return sent;
    }

    bool NetworkManager::broadcast(NetworkMessage& message) {
        if (!m_running) return false;

        Packet packet;
        auto serialized = message.serialize();
        if (m_encryptionEnabled) {
            encryptData(serialized);
        }
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
        NetworkStats stats = m_stats;
        stats.latencyMs = m_lastLatencyMs;
        stats.jitterMs = m_lastJitterMs;
        {
            std::lock_guard<std::mutex> poolLock(m_poolMutex);
            stats.pooledConnections = static_cast<uint32_t>(m_connectionPool.size());
        }
        return stats;
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

        if (m_autoReconnect && !m_connected && m_running && !m_lastHost.empty()) {
            attemptReconnection();
        }

        manageConnectionPool();

        checkTimeouts();
        resendUnacknowledged();
    }

    void NetworkManager::heartbeatLoop() {
        while (m_threadsRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (!m_running) continue;

            if (m_connected) {
                uint64_t now = getCurrentTimeMs();
                if (now - m_lastHeartbeatTime >= m_heartbeatInterval) {
                    sendHeartbeat();
                    m_lastHeartbeatTime = now;
                }
                if (m_latencyJitterEnabled) {
                    measureLatency();
                }
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

            if (m_encryptionEnabled) {
                decryptData(buffer);
            }

            NetworkMessage msg;
            if (msg.deserialize(buffer)) {
                {
                    std::lock_guard<std::mutex> lock(m_statsMutex);
                    m_stats.bytesReceived += size + 4;
                    m_stats.packetsReceived++;
                    m_stats.messagesReceived++;
                    m_bytesReceivedThisInterval += size + 4;
                    if (m_encryptionEnabled) {
                        m_stats.totalBytesDecrypted += buffer.size();
                    }
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

            if (m_encryptionEnabled) {
                decryptData(buffer);
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
                } else if (msg.getType() == MessageType::Pong) {
                    if (m_latencyJitterEnabled) {
                        uint64_t rtt = getCurrentTimeMs() - msg.getID();
                        double currentLatency = static_cast<double>(rtt);
                        updateJitter(currentLatency);
                    }
                } else {
                    NetworkMessage ack = NetworkMessage::createACK(msg.getID());
                    send(ack);

                    {
                        std::lock_guard<std::mutex> lock(m_statsMutex);
                        m_stats.bytesReceived += size + 4;
                        m_stats.packetsReceived++;
                        m_stats.messagesReceived++;
                        m_bytesReceivedThisInterval += size + 4;
                        if (m_encryptionEnabled) {
                            m_stats.totalBytesDecrypted += buffer.size();
                        }
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

        if (now - m_lastHeartbeatTime >= m_timeoutInterval) {
            m_connected = false;
            m_tcpSocket.disconnect();

            std::lock_guard<std::mutex> statsLock(m_statsMutex);
            m_stats.packetLoss++;
            if (m_stats.activeConnections > 0) m_stats.activeConnections--;

            if (m_handler) {
                NetworkMessage timeoutMsg;
                timeoutMsg.setType(MessageType::Disconnect);
                timeoutMsg.setID(0);
                m_handler(timeoutMsg);
            }
            return;
        }

        std::lock_guard<std::mutex> lock(m_ackMutex);
        for (auto& entry : m_lastSendTime) {
            if (now - entry.second >= m_timeoutInterval) {
                m_connected = false;
                m_tcpSocket.disconnect();

                std::lock_guard<std::mutex> statsLock(m_statsMutex);
                m_stats.packetLoss++;
                if (m_stats.activeConnections > 0) m_stats.activeConnections--;

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
                    {
                        std::lock_guard<std::mutex> statsLock(m_statsMutex);
                        m_stats.packetLoss++;
                    }
                    continue;
                }

                if (m_bandwidthLimitBytesPerSec > 0) {
                    if (now - m_bandwidthIntervalStart >= 1000) {
                        m_bandwidthUsedThisSecond = 0;
                        m_bandwidthIntervalStart = now;
                    }
                }

                Packet packet;
                auto serialized = entry.second.serialize();
                if (m_encryptionEnabled) {
                    encryptData(serialized);
                }
                packet.writeInt32(static_cast<int32_t>(serialized.size()));
                for (auto byte : serialized) {
                    packet.writeInt8(static_cast<int8_t>(byte));
                }

                if (m_bandwidthLimitBytesPerSec > 0) {
                    size_t packetSize = serialized.size() + 4;
                    if (m_bandwidthUsedThisSecond + packetSize > m_bandwidthLimitBytesPerSec) {
                        continue;
                    }
                    m_bandwidthUsedThisSecond += packetSize;
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

    void NetworkManager::attemptReconnection() {
        uint64_t now = getCurrentTimeMs();
        if (now - m_lastReconnectTime < m_reconnectDelayMs) return;
        if (m_reconnectionAttempts >= m_maxReconnectionAttempts) return;

        m_lastReconnectTime = now;
        m_reconnectionAttempts++;

        bool success = m_tcpSocket.connect(m_lastHost, m_lastPort, sf::seconds(3));
        if (success) {
            m_connected = true;
            m_reconnectionAttempts = 0;

            NetworkMessage reconnectMsg;
            reconnectMsg.setType(MessageType::Connect);
            reconnectMsg.setID(getNextMessageID());
            send(reconnectMsg);

            std::lock_guard<std::mutex> lock(m_statsMutex);
            m_stats.activeConnections++;
        }
    }

    void NetworkManager::manageConnectionPool() {
        std::lock_guard<std::mutex> lock(m_poolMutex);
        uint64_t now = getCurrentTimeMs();

        m_connectionPool.erase(
            std::remove_if(m_connectionPool.begin(), m_connectionPool.end(),
                [now](const PooledConnection& pc) {
                    return !pc.inUse && (now - pc.createdAt >= POOL_CONNECTION_TTL_MS);
                }),
            m_connectionPool.end()
        );

        while (m_connectionPool.size() < m_maxPoolSize) {
            PooledConnection pc;
            pc.host = m_lastHost;
            pc.port = m_lastPort;
            pc.inUse = false;
            pc.lastUsed = now;
            pc.createdAt = now;
            m_connectionPool.push_back(std::move(pc));
        }
    }

    void NetworkManager::encryptData(std::vector<uint8_t>& data) {
        if (!m_encryptionEnabled || m_encryptionKey.empty()) return;
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] ^= m_encryptionKey[i % m_encryptionKey.size()];
        }
    }

    void NetworkManager::decryptData(std::vector<uint8_t>& data) {
        encryptData(data);
    }

    void NetworkManager::measureLatency() {
        NetworkMessage ping = NetworkMessage::createPing();
        uint32_t id = getNextMessageID();
        ping.setID(id);
        send(ping);
    }

    void NetworkManager::updateJitter(double currentLatency) {
        m_lastLatencyMs = currentLatency;
        m_latencyHistory.push_back(currentLatency);
        if (m_latencyHistory.size() > LATENCY_HISTORY_SIZE) {
            m_latencyHistory.pop_front();
        }

        if (m_latencyHistory.size() >= 2) {
            double sum = 0.0;
            double prev = m_latencyHistory[0];
            for (size_t i = 1; i < m_latencyHistory.size(); ++i) {
                sum += std::abs(m_latencyHistory[i] - prev);
                prev = m_latencyHistory[i];
            }
            m_lastJitterMs = sum / (m_latencyHistory.size() - 1);
        }

        std::lock_guard<std::mutex> lock(m_statsMutex);
        m_stats.latencyMs = currentLatency;
        m_stats.jitterMs = m_lastJitterMs;
    }

    void NetworkManager::setEncryptionKey(const std::vector<uint8_t>& key) {
        m_encryptionKey = key;
    }

    void NetworkManager::enableEncryption(bool enable) {
        m_encryptionEnabled = enable;
    }

    bool NetworkManager::isEncryptionEnabled() const {
        return m_encryptionEnabled;
    }

    void NetworkManager::setBandwidthLimit(uint64_t bytesPerSecond) {
        m_bandwidthLimitBytesPerSec = bytesPerSecond;
    }

    uint64_t NetworkManager::getBandwidthLimit() const {
        return m_bandwidthLimitBytesPerSec;
    }

    void NetworkManager::setAutoReconnect(bool enable) {
        m_autoReconnect = enable;
        if (!enable) m_reconnectionAttempts = 0;
    }

    bool NetworkManager::isAutoReconnectEnabled() const {
        return m_autoReconnect;
    }

    void NetworkManager::setMaxReconnectionAttempts(uint32_t attempts) {
        m_maxReconnectionAttempts = attempts;
    }

    uint32_t NetworkManager::getMaxReconnectionAttempts() const {
        return m_maxReconnectionAttempts;
    }

    void NetworkManager::setConnectionPoolSize(uint32_t size) {
        m_maxPoolSize = size;
    }

    uint32_t NetworkManager::getConnectionPoolSize() const {
        return m_maxPoolSize;
    }

    uint32_t NetworkManager::getActiveConnectionCount() const {
        std::lock_guard<std::mutex> lock(m_statsMutex);
        return m_stats.activeConnections;
    }

    void NetworkManager::enableLatencyJitterMeasurement(bool enable) {
        m_latencyJitterEnabled = enable;
    }

    bool NetworkManager::isLatencyJitterMeasurementEnabled() const {
        return m_latencyJitterEnabled;
    }

    uint64_t NetworkManager::getCurrentTimeMs() {
        return static_cast<uint64_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count()
        );
    }

}
