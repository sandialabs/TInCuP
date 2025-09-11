/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

/**
 * Comprehensive TInCuP Networking Serialization Example
 * 
 * This example demonstrates:
 * 1. CPO-based serialization with multiple backends
 * 2. Built-in support for fundamental types and STL containers  
 * 3. User-defined type serialization via tag_invoke
 * 4. Third-party type support via cpo_impl specialization
 * 5. Enhanced diagnostics when serialization fails
 * 6. Multi-format serialization (binary and JSON)
 */

#include "binary_backend.hpp"
#include "json_backend.hpp"
#include "user_types.hpp"
#include "third_party_support.hpp"

#include <iostream>
#include <chrono>
#include <cassert>

using namespace networking;
using namespace networking::examples;
using namespace networking::third_party;

// =============================================================================
// Test utility functions
// =============================================================================

template<typename T>
void test_binary_roundtrip(const T& original, const std::string& type_name) {
    std::cout << "Testing binary serialization roundtrip for " << type_name << "...\n";
    
    // Serialize
    binary_writer writer;
    serialize(writer, original);
    
    std::cout << "  Serialized size: " << writer.size() << " bytes\n";
    
    // Deserialize
    binary_reader reader(writer.data());
    T deserialized;
    deserialize(reader, deserialized);
    
    // Verify
    assert(original == deserialized);
    std::cout << "  ✓ Roundtrip successful!\n\n";
}

template<typename T>
void test_json_serialization(const T& value, const std::string& type_name) {
    std::cout << "Testing JSON serialization for " << type_name << "...\n";
    
    json_writer writer;
    serialize(writer, value);
    
    std::cout << "JSON output:\n" << writer.str() << "\n\n";
}

// =============================================================================
// Main demonstration
// =============================================================================

int main() {
    std::cout << "=== TInCuP Networking Serialization Example ===\n\n";

    // -------------------------------------------------------------------------
    // 1. Fundamental types and STL containers
    // -------------------------------------------------------------------------
    
    std::cout << "1. FUNDAMENTAL TYPES AND STL CONTAINERS\n";
    std::cout << "----------------------------------------\n";
    
    // Test basic types
    test_binary_roundtrip(42, "int");
    test_binary_roundtrip(3.14159f, "float");
    test_binary_roundtrip(std::string("Hello, TInCuP!"), "string");
    
    // Test containers
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    test_binary_roundtrip(numbers, "vector<int>");
    
    std::vector<std::string> words = {"networking", "serialization", "CPO"};
    test_binary_roundtrip(words, "vector<string>");

    // -------------------------------------------------------------------------
    // 2. User-defined types via tag_invoke
    // -------------------------------------------------------------------------
    
    std::cout << "2. USER-DEFINED TYPES (via tag_invoke)\n";
    std::cout << "---------------------------------------\n";
    
    Point2D point(10.5f, 20.3f);
    test_binary_roundtrip(point, "Point2D");
    test_json_serialization(point, "Point2D");
    
    PlayerStats stats{25, 100, 50, 1250.75f};
    test_binary_roundtrip(stats, "PlayerStats");
    test_json_serialization(stats, "PlayerStats");
    
    Player player;
    player.name = "TestPlayer";
    player.position = Point2D(100.0f, 200.0f);
    player.stats = PlayerStats{30, 150, 80, 2500.5f};
    player.inventory = {"sword", "shield", "potion"};
    
    test_binary_roundtrip(player, "Player");
    test_json_serialization(player, "Player");
    
    // Test network messages
    ChatMessage chat_msg{"Alice", "Hello from TInCuP!"};
    NetworkMessage net_msg(chat_msg);
    
    test_binary_roundtrip(net_msg, "NetworkMessage (Chat)");
    test_json_serialization(net_msg, "NetworkMessage (Chat)");

    // -------------------------------------------------------------------------
    // 3. Third-party types via cpo_impl specialization
    // -------------------------------------------------------------------------
    
    std::cout << "3. THIRD-PARTY TYPES (via cpo_impl specialization)\n";
    std::cout << "---------------------------------------------------\n";
    
    // UUID type (simulated third-party)
    external_lib::UUID session_uuid(0x123456789ABCDEF0ULL, 0xFEDCBA9876543210ULL);
    test_binary_roundtrip(session_uuid, "external_lib::UUID");
    
    // Fixed buffer (template specialization)
    external_lib::FixedBuffer<256> buffer;
    buffer.used_size = 10;
    std::string test_data = "test_data!";
    std::copy(test_data.begin(), test_data.end(), buffer.data.begin());
    test_binary_roundtrip(buffer, "external_lib::FixedBuffer<256>");
    
    // Standard library types
    std::complex<float> complex_num(3.0f, 4.0f);
    test_binary_roundtrip(complex_num, "std::complex<float>");
    
    auto now = std::chrono::system_clock::now();
    test_binary_roundtrip(now, "std::chrono::system_clock::time_point");
    
    // Combined type using all third-party types
    GameSession game_session;
    game_session.session_id = session_uuid;
    game_session.start_time = now;
    game_session.server_coordinates = complex_num;
    game_session.session_data = buffer;
    
    test_binary_roundtrip(game_session, "GameSession (with third-party types)");

    // -------------------------------------------------------------------------
    // 4. Format compatibility demonstration  
    // -------------------------------------------------------------------------
    
    std::cout << "4. MULTI-FORMAT SERIALIZATION DEMO\n";
    std::cout << "-----------------------------------\n";
    
    std::cout << "Same Player data in different formats:\n\n";
    
    // Binary format
    binary_writer bin_writer;
    serialize(bin_writer, player);
    std::cout << "Binary format: " << bin_writer.size() << " bytes\n";
    
    // JSON format  
    json_writer json_writer_obj;
    serialize(json_writer_obj, player);
    std::cout << "JSON format:\n" << json_writer_obj.str() << "\n";

    // -------------------------------------------------------------------------
    // 5. Error handling demonstration (commented out to avoid compilation errors)
    // -------------------------------------------------------------------------
    
    std::cout << "5. ERROR HANDLING AND DIAGNOSTICS\n";
    std::cout << "----------------------------------\n";
    std::cout << "The following would generate helpful error messages:\n";
    std::cout << "// Uncomment to see enhanced diagnostics in action:\n";
    std::cout << "\n";
    std::cout << "/*\n";
    std::cout << "struct UnsupportedType { int x; };\n"; 
    std::cout << "UnsupportedType unsupported;\n";
    std::cout << "serialize(writer, unsupported);  // Clear diagnostic message\n";
    std::cout << "\n";
    std::cout << "std::unique_ptr<Player> player_ptr = std::make_unique<Player>();\n";
    std::cout << "serialize(writer, player_ptr);  // \"Pointer detected - try *ptr\"\n";
    std::cout << "*/\n";
    
    std::cout << "\n=== All tests completed successfully! ===\n";
    std::cout << "\nKey takeaways:\n";
    std::cout << "• Single CPO interface works with multiple backends\n";
    std::cout << "• Users extend via tag_invoke for types they control\n";
    std::cout << "• Third-party types supported via cpo_impl specialization\n";
    std::cout << "• Enhanced diagnostics guide users to correct usage\n";
    std::cout << "• Same data can serialize to multiple formats seamlessly\n";
    
    return 0;
}