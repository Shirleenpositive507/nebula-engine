# Networking Guide

## Architecture

Nebula Networking supports both TCP and UDP protocols for client-server communication.

## Setting Up a Server

```cpp
#include <Nebula/Networking.h>

nebula::net::Server server;

server.onClientConnected = [](net::Connection& client) {
    std::cout << "Client connected: " << client.getId() << "\n";
    server.broadcast(net::Packet::make("PlayerJoined", client.getId()));
};

server.onClientDisconnected = [](net::Connection& client) {
    std::cout << "Client disconnected: " << client.getId() << "\n";
};

server.onPacketReceived = [](net::Connection& client, net::Packet& packet) {
    if (packet.getType() == "PlayerInput") {
        // Handle input
        server.broadcast(packet, client); // Relay to others
    }
};

server.start(27015); // Port
```

## Setting Up a Client

```cpp
nebula::net::Client client;

client.onConnected = []() {
    std::cout << "Connected to server!\n";
};

client.onDisconnected = [](net::DisconnectReason reason) {
    std::cout << "Disconnected: " << reason << "\n";
};

client.onPacketReceived = [](net::Packet& packet) {
    if (packet.getType() == "WorldState") {
        auto state = packet.read<WorldState>();
        // Update local game state
    }
};

client.connect("127.0.0.1", 27015);
```

## Message Handling

```cpp
// Sending packets
net::Packet packet("PlayerMove");
packet.write(playerId);
packet.write(position);
packet.write(velocity);
server.send(client, packet);

// Reliable vs unreliable
packet.setReliability(net::Reliability::Reliable);   // TCP-like
packet.setReliability(net::Reliability::Unreliable); // UDP-like

// Packet types
packet.setType("ChatMessage");
packet.setType("GameState");
```

## Reliability

```cpp
// Reliable ordered (default)
packet.setChannel(net::Channel::Default);

// Unreliable (for position updates)
packet.setChannel(net::Channel::Unreliable);

// Reliable unordered
packet.setChannel(net::Channel::ReliableUnordered);

// Sequenced (reliable, latest only)
packet.setChannel(net::Channel::Sequenced);
```

## Connection Management

```cpp
// Server: manage connected clients
for (auto& [id, client] : server.getClients()) {
    if (client.getPing() > 500) {
        server.disconnect(client, "Timeout");
    }
}

// Client: check connection status
if (client.getPing() > 1000) {
    client.disconnect();
    client.reconnect();
}

// Encryption
server.enableEncryption("shared_secret_key");
client.enableEncryption("shared_secret_key");
```
