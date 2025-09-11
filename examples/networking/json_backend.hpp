/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

/**
 * JSON serialization backend implementation
 * 
 * This demonstrates how the same CPO interface can work with completely
 * different serialization formats by just changing the backend.
 */

#pragma once

#include "serialize.hpp"
#include "user_types.hpp"
#include <sstream>
#include <string>
#include <iomanip>

namespace networking {

// =============================================================================
// JSON Writer - Simple JSON serialization backend  
// =============================================================================

class json_writer {
private:
    std::ostringstream stream_;
    int indent_level_;
    bool need_comma_;
    
    void write_indent() {
        for (int i = 0; i < indent_level_; ++i) {
            stream_ << "  ";
        }
    }
    
    void write_separator() {
        if (need_comma_) {
            stream_ << ",\n";
        } else {
            need_comma_ = true;
        }
        write_indent();
    }

public:
    json_writer() : indent_level_(0), need_comma_(false) {}
    
    void begin_object() {
        stream_ << "{\n";
        indent_level_++;
        need_comma_ = false;
    }
    
    void end_object() {
        stream_ << "\n";
        indent_level_--;
        write_indent();
        stream_ << "}";
        need_comma_ = true;
    }
    
    void begin_array() {
        stream_ << "[\n";
        indent_level_++;
        need_comma_ = false;
    }
    
    void end_array() {
        stream_ << "\n";
        indent_level_--;
        write_indent();
        stream_ << "]";
        need_comma_ = true;
    }
    
    void write_key(std::string_view key) {
        write_separator();
        stream_ << "\"" << key << "\": ";
    }
    
    void write_value(std::string_view value) {
        stream_ << value;
    }
    
    void write_key_value(std::string_view key, std::string_view value) {
        write_key(key);
        write_value(value);
    }
    
    void write_array_element(std::string_view value) {
        write_separator();
        stream_ << value;
    }
    
    std::string str() const { return stream_.str(); }
};

// =============================================================================  
// JSON serialization for fundamental types
// =============================================================================

// Numeric types
template<typename T>
requires std::is_arithmetic_v<T> && (!std::is_same_v<T, bool>)
void tag_invoke(serialize_ftor, json_writer& writer, const T& value) {
    std::ostringstream ss;
    if constexpr (std::is_floating_point_v<T>) {
        ss << std::fixed << std::setprecision(6) << value;
    } else {
        ss << value;
    }
    writer.write_value(ss.str());
}

// Boolean
inline void tag_invoke(serialize_ftor, json_writer& writer, bool value) {
    writer.write_value(value ? "true" : "false");
}

// String  
inline void tag_invoke(serialize_ftor, json_writer& writer, const std::string& str) {
    std::ostringstream ss;
    ss << "\"";
    // Simple JSON string escaping (in real code you'd handle all escape sequences)
    for (char c : str) {
        switch (c) {
            case '"': ss << "\\\""; break;
            case '\\': ss << "\\\\"; break; 
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default: ss << c; break;
        }
    }
    ss << "\"";
    writer.write_value(ss.str());
}

// Vector (as JSON array)
template<typename T>
requires tincup::invocable_c<serialize_ftor, json_writer&, const T&>
void tag_invoke(serialize_ftor, json_writer& writer, const std::vector<T>& vec) {
    writer.begin_array();
    for (const auto& element : vec) {
        // Use write_array_element which handles comma separation
        std::ostringstream element_stream;
        json_writer element_writer;
        serialize(element_writer, element);
        writer.write_array_element(element_writer.str());
    }
    writer.end_array();
}

} // namespace networking

// =============================================================================
// JSON serialization for user-defined types (showing structured output)
// =============================================================================

namespace networking::examples {

// Point2D as JSON object
inline void tag_invoke(serialize_ftor, json_writer& writer, const Point2D& point) {
    writer.begin_object();
    writer.write_key_value("x", std::to_string(point.x));
    writer.write_key_value("y", std::to_string(point.y));
    writer.end_object();
}

// PlayerStats as JSON object
inline void tag_invoke(serialize_ftor, json_writer& writer, const PlayerStats& stats) {
    writer.begin_object();
    writer.write_key_value("level", std::to_string(stats.level));
    writer.write_key_value("health", std::to_string(stats.health)); 
    writer.write_key_value("mana", std::to_string(stats.mana));
    writer.write_key_value("experience", std::to_string(stats.experience));
    writer.end_object();
}

// Player as comprehensive JSON object
inline void tag_invoke(serialize_ftor, json_writer& writer, const Player& player) {
    writer.begin_object();
    
    // Simple fields
    writer.write_key("name");
    serialize(writer, player.name);
    
    // Nested object
    writer.write_key("position");  
    serialize(writer, player.position);
    
    // Another nested object
    writer.write_key("stats");
    serialize(writer, player.stats);
    
    // Array field
    writer.write_key("inventory");
    serialize(writer, player.inventory);
    
    writer.end_object();
}

// Chat message 
inline void tag_invoke(serialize_ftor, json_writer& writer, const ChatMessage& msg) {
    writer.begin_object();
    writer.write_key("sender");
    serialize(writer, msg.sender);
    writer.write_key("message");  
    serialize(writer, msg.message);
    writer.end_object();
}

// Network message with type discrimination
inline void tag_invoke(serialize_ftor, json_writer& writer, const NetworkMessage& msg) {
    writer.begin_object();
    
    // Write message type as string for readability
    writer.write_key("type");
    switch (msg.type) {
        case MessageType::PING:
            writer.write_value("\"ping\"");
            writer.write_key("data");
            writer.begin_object();
            writer.write_key_value("timestamp", std::to_string(msg.ping.timestamp));
            writer.end_object();
            break;
        case MessageType::PLAYER_UPDATE:
            writer.write_value("\"player_update\"");
            writer.write_key("data");
            serialize(writer, msg.player_update);
            break;
        case MessageType::CHAT_MESSAGE:
            writer.write_value("\"chat_message\"");
            writer.write_key("data");
            serialize(writer, msg.chat);
            break;
    }
    
    writer.end_object();
}

} // namespace networking::examples