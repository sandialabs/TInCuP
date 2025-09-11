/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

/**
 * Example user-defined types demonstrating tag_invoke serialization
 */

#pragma once

#include "binary_backend.hpp"
#include <array>

namespace networking::examples {

// =============================================================================
// Example 1: Simple 2D Point
// =============================================================================

struct Point2D {
    float x, y;
    
    Point2D() = default;
    Point2D(float x_, float y_) : x(x_), y(y_) {}
    
    bool operator==(const Point2D& other) const {
        return x == other.x && y == other.y;
    }
};

// User-provided serialization via tag_invoke
inline void tag_invoke(serialize_ftor, binary_writer& writer, const Point2D& point) {
    serialize(writer, point.x, point.y);  // Serialize both coordinates
}

inline void tag_invoke(deserialize_ftor, binary_reader& reader, Point2D& point) {
    deserialize(reader, point.x);
    deserialize(reader, point.y);
}

// =============================================================================
// Example 2: Player data with nested types  
// =============================================================================

struct PlayerStats {
    uint32_t level;
    uint32_t health;
    uint32_t mana;
    float experience;
    
    bool operator==(const PlayerStats& other) const {
        return level == other.level && health == other.health && 
               mana == other.mana && experience == other.experience;
    }
};

// Serialization for PlayerStats
inline void tag_invoke(serialize_ftor, binary_writer& writer, const PlayerStats& stats) {
    serialize(writer, stats.level, stats.health, stats.mana, stats.experience);
}

inline void tag_invoke(deserialize_ftor, binary_reader& reader, PlayerStats& stats) {
    deserialize(reader, stats.level);
    deserialize(reader, stats.health);  
    deserialize(reader, stats.mana);
    deserialize(reader, stats.experience);
}

struct Player {
    std::string name;
    Point2D position;
    PlayerStats stats;
    std::vector<std::string> inventory;
    
    bool operator==(const Player& other) const {
        return name == other.name && position == other.position && 
               stats == other.stats && inventory == other.inventory;
    }
};

// Serialization for Player - demonstrates nested type serialization
inline void tag_invoke(serialize_ftor, binary_writer& writer, const Player& player) {
    serialize(writer, player.name);      // String (built-in)
    serialize(writer, player.position);  // Point2D (user-defined)  
    serialize(writer, player.stats);     // PlayerStats (user-defined)
    serialize(writer, player.inventory); // Vector<string> (STL container)
}

inline void tag_invoke(deserialize_ftor, binary_reader& reader, Player& player) {
    deserialize(reader, player.name);
    deserialize(reader, player.position);
    deserialize(reader, player.stats);
    deserialize(reader, player.inventory);
}

// =============================================================================
// Example 3: Network Message with variant-like behavior
// =============================================================================

enum class MessageType : uint8_t {
    PING = 1,
    PLAYER_UPDATE = 2, 
    CHAT_MESSAGE = 3
};

struct PingMessage {
    uint64_t timestamp;
    
    bool operator==(const PingMessage& other) const {
        return timestamp == other.timestamp;
    }
};

struct ChatMessage {
    std::string sender;
    std::string message;
    
    bool operator==(const ChatMessage& other) const {
        return sender == other.sender && message == other.message;
    }
};

// A message wrapper that can hold different message types
class NetworkMessage {
public:
    MessageType type;
    
    // Union-like storage (simplified for example)
    union {
        PingMessage ping;
        Player player_update;
        ChatMessage chat;
    };
    
    NetworkMessage() : type(MessageType::PING), ping{} {}
    
    NetworkMessage(const PingMessage& msg) : type(MessageType::PING), ping(msg) {}
    NetworkMessage(const Player& msg) : type(MessageType::PLAYER_UPDATE), player_update(msg) {}  
    NetworkMessage(const ChatMessage& msg) : type(MessageType::CHAT_MESSAGE), chat(msg) {}
    
    ~NetworkMessage() {
        // Proper cleanup based on type would go here in real code
    }
    
    bool operator==(const NetworkMessage& other) const {
        if (type != other.type) return false;
        switch (type) {
            case MessageType::PING:
                return ping == other.ping;
            case MessageType::PLAYER_UPDATE:
                return player_update == other.player_update;
            case MessageType::CHAT_MESSAGE:
                return chat == other.chat;
        }
        return false;
    }
};

// Message type serialization  
inline void tag_invoke(serialize_ftor, binary_writer& writer, const PingMessage& msg) {
    serialize(writer, msg.timestamp);
}

inline void tag_invoke(deserialize_ftor, binary_reader& reader, PingMessage& msg) {
    deserialize(reader, msg.timestamp);
}

inline void tag_invoke(serialize_ftor, binary_writer& writer, const ChatMessage& msg) {
    serialize(writer, msg.sender, msg.message);
}

inline void tag_invoke(deserialize_ftor, binary_reader& reader, ChatMessage& msg) {
    deserialize(reader, msg.sender);
    deserialize(reader, msg.message);
}

// NetworkMessage serialization with type dispatch
inline void tag_invoke(serialize_ftor, binary_writer& writer, const NetworkMessage& msg) {
    serialize(writer, msg.type);  // Serialize type first
    
    switch (msg.type) {
        case MessageType::PING:
            serialize(writer, msg.ping);
            break;
        case MessageType::PLAYER_UPDATE:
            serialize(writer, msg.player_update);
            break;
        case MessageType::CHAT_MESSAGE:
            serialize(writer, msg.chat);
            break;
    }
}

inline void tag_invoke(deserialize_ftor, binary_reader& reader, NetworkMessage& msg) {
    deserialize(reader, msg.type);  // Read type first
    
    switch (msg.type) {
        case MessageType::PING:
            deserialize(reader, msg.ping);
            break;
        case MessageType::PLAYER_UPDATE:
            deserialize(reader, msg.player_update);
            break;
        case MessageType::CHAT_MESSAGE:
            deserialize(reader, msg.chat);
            break;
    }
}

} // namespace networking::examples