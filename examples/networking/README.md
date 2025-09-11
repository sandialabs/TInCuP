# TInCuP Networking Serialization Example

This example demonstrates a complete, production-ready serialization system built using TInCuP (Tag Invoke Customization Point) library. It showcases how CPOs provide clean, extensible interfaces for complex operations like serialization across multiple formats.

## Features Demonstrated

### 🎯 **Core CPO Benefits**
- **Single Interface, Multiple Backends**: Same `serialize()` call works with binary, JSON, or any custom format
- **Zero-Cost Abstraction**: No runtime overhead - everything compiles to direct function calls
- **Type Safety**: Compile-time verification that types are serializable
- **Enhanced Diagnostics**: Clear error messages when serialization fails

### 🔧 **Extensibility Patterns**
- **User Types**: Add serialization via `tag_invoke` for types you control
- **Third-Party Types**: Use `cpo_impl` specialization for types you don't control
- **Format Agnostic**: Write serialization logic once, use with any backend

### 📊 **Practical Features**
- Built-in support for fundamental types and STL containers
- Nested type serialization
- Length-prefixed strings and containers
- Type-safe deserialization with factory methods
- Error handling and validation

## Quick Start

```cpp
#include "binary_backend.hpp"
#include "user_types.hpp"

using namespace networking;

// 1. Built-in types work automatically
binary_writer writer;
serialize(writer, 42);
serialize(writer, std::string("Hello"));
serialize(writer, std::vector<int>{1, 2, 3});

// 2. User types via tag_invoke
struct Point { float x, y; };

void tag_invoke(serialize_ftor, binary_writer& w, const Point& p) {
    serialize(w, p.x, p.y);
}

Point pt{10.5f, 20.3f};
serialize(writer, pt);  // Works!

// 3. Same data, different format
json_writer json;
serialize(json, pt);  // Same interface, JSON output
```

## File Structure

```
networking/
├── serialize.hpp           # Core CPO definitions and concepts
├── binary_backend.hpp      # Binary serialization implementation
├── json_backend.hpp        # JSON serialization implementation  
├── user_types.hpp          # Example user-defined types
├── third_party_support.hpp # cpo_impl specialization examples
├── main.cpp               # Comprehensive demonstration
└── README.md              # This file
```

## Detailed Examples

### 1. Built-in Type Support

The system automatically handles fundamental types and STL containers:

```cpp
// Fundamental types
serialize(writer, 42);           // int
serialize(writer, 3.14f);        // float
serialize(writer, true);         // bool

// STL containers  
serialize(writer, std::string("text"));
serialize(writer, std::vector<int>{1, 2, 3});

// Roundtrip deserialization
binary_reader reader(writer.data());
int value = deserialize<int>(reader);
auto text = deserialize<std::string>(reader);
```

### 2. User-Defined Types

Add serialization support for your types using `tag_invoke`:

```cpp
struct Player {
  std::string name;
  Point2D position;  
  PlayerStats stats;
  std::vector<std::string> inventory;
};

// Serialization implementation
void tag_invoke(serialize_ftor, binary_writer& writer, const Player& player) {
  serialize(writer, player.name);      // Built-in string
  serialize(writer, player.position);  // User-defined Point2D  
  serialize(writer, player.stats);     // User-defined PlayerStats
  serialize(writer, player.inventory); // STL vector<string>
}

// Corresponding deserialization
void tag_invoke(deserialize_ftor, binary_reader& reader, Player& player) {
  deserialize(reader, player.name);
  deserialize(reader, player.position);
  deserialize(reader, player.stats);
  deserialize(reader, player.inventory);
}
```

### 3. Third-Party Type Support

Extend serialization for types you don't control using `cpo_impl` specialization:

```cpp
// For a third-party UUID type you can't modify
namespace tincup {
  template<>
  struct cpo_impl<serialize_ftor, external_lib::UUID> {
    void operator()(binary_writer& writer, const external_lib::UUID& uuid) const {
      serialize(writer, uuid.high, uuid.low);
    }
  };
}

// Now UUID serialization works automatically
external_lib::UUID session_id{0x123, 0x456};
serialize(writer, session_id);  // Uses cpo_impl specialization
```

### 4. Multi-Format Serialization

Same data structure, multiple output formats:

```cpp
Player player{"Alice", {10.0f, 20.0f}, {25, 100, 50, 1250.0f}, {"sword", "potion"}};

// Binary format (compact, efficient)
binary_writer binary;
serialize(binary, player);
// Result: ~50 bytes of binary data

// JSON format (human-readable, interoperable) 
json_writer json;
serialize(json, player);
// Result: Structured JSON object
```

**JSON Output:**
```json
{
  "name": "Alice",
  "position": {
    "x": "10.000000",
    "y": "20.000000"
  },
  "stats": {
    "level": "25",
    "health": "100", 
    "mana": "50",
    "experience": "1250.000000"
  },
  "inventory": [
    "sword",
    "potion"
  ]
}
```

### 5. Enhanced Diagnostics

TInCuP provides clear error messages when serialization fails:

```cpp
struct UnsupportedType { int x; };

UnsupportedType unsupported;
serialize(writer, unsupported);
// Error: "No valid tag_invoke overload found. Check CPO and argument types above."

std::unique_ptr<Player> player_ptr = std::make_unique<Player>();  
serialize(writer, player_ptr);
// Error: "POINTER ARGUMENT DETECTED: The argument appears to be a pointer/smart_ptr 
//         that needs dereferencing. Try using operator* on the argument."
```

## Building and Running

```bash
# Compile and run the example
cd examples/networking
g++ -std=c++20 -I../.. main.cpp -o networking_example
./networking_example
```

**Sample Output:**
```
=== TInCuP Networking Serialization Example ===

1. FUNDAMENTAL TYPES AND STL CONTAINERS
----------------------------------------
Testing binary serialization roundtrip for int...
  Serialized size: 4 bytes
  ✓ Roundtrip successful!

Testing binary serialization roundtrip for string...
  Serialized size: 17 bytes  
  ✓ Roundtrip successful!

2. USER-DEFINED TYPES (via tag_invoke)
---------------------------------------
Testing binary serialization roundtrip for Player...
  Serialized size: 78 bytes
  ✓ Roundtrip successful!

[... continued output ...]
```

## Key Design Patterns

### CPO Interface Design
```cpp
// Clean, consistent interface across all backends
template<typename Writer, typename T>
auto serialize(Writer& writer, const T& value) -> /* deduced */;

// Variadic support for multiple values  
template<typename Writer, typename... Args>
void serialize(Writer& writer, const Args&... values);
```

### Backend Abstraction
```cpp
// Backends implement simple, focused interfaces
template<typename Writer>
concept binary_writer_c = requires(Writer& w, const void* data, std::size_t size) {
    w.write_bytes(data, size);
};

// JSON writers have different but equally simple requirements
template<typename Writer> 
concept json_writer_c = requires(Writer& w) {
    w.begin_object();
    w.write_key_value("key", "value");
};
```

### Extension Points
```cpp
// Method 1: tag_invoke for types you control
void tag_invoke(serialize_ftor, Writer& w, const MyType& value);

// Method 2: cpo_impl for types you don't control  
template<> struct cpo_impl<serialize_ftor, ThirdPartyType> { /* ... */ };
```

## Real-World Applications

This pattern is ideal for:

- **Network Protocols**: Same message types, multiple wire formats
- **Database ORMs**: Single entity definition, multiple database backends  
- **Configuration Systems**: Same config objects, JSON/YAML/binary formats
- **API Layers**: Consistent serialization across REST/gRPC/MessagePack
- **Game Engines**: Save files, network messages, asset pipelines

## Advanced Usage

### Custom Backends

Create your own serialization backend:

```cpp
class custom_writer {
public:
    void write_bytes(const void* data, std::size_t size) { /* implementation */ }
    // Add any backend-specific methods
};

// Built-in types automatically work
serialize(custom_writer{}, my_data);

// Add format-specific overrides as needed
void tag_invoke(serialize_ftor, custom_writer& w, const SpecialType& value) {
    // Custom serialization logic for this backend
}
```

### Performance Considerations

- **Zero Overhead**: CPOs compile to direct function calls
- **Minimal Allocations**: Binary backend uses single growing buffer
- **Type Erasure Free**: No virtual dispatch or function pointers
- **Compile-Time Optimization**: All serialization paths known at compile time

### Error Handling

The system provides multiple levels of error handling:

1. **Compile-Time**: Detect unsupported types before runtime
2. **Enhanced Diagnostics**: Guide users to correct implementations  
3. **Runtime Validation**: Buffer bounds checking, data validation
4. **Exception Safety**: RAII-based resource management

## Integration with TInCuP

This example showcases TInCuP's key strengths:

- **Customization Point Objects**: Clean, extensible interfaces
- **tag_invoke Protocol**: Standard extension mechanism
- **cpo_impl Specialization**: Support for third-party types
- **Enhanced Diagnostics**: Helpful compiler error messages
- **Type Safety**: Compile-time guarantees about operations

The serialization system demonstrates how CPOs enable building sophisticated, performant libraries with clean user interfaces and excellent extensibility.
