# TInCuP Serialize Example

This example demonstrates the basic concepts of TInCuP's Customization Point Objects (CPOs) through a simple JSON serialization system. It showcases the fundamental patterns that enable the more sophisticated networking serialization example.

## Overview

The serialize example shows:
- **Basic CPO Implementation**: How to create a customization point using `tag_invoke`
- **User-Defined Type Extension**: Adding serialization for your own types 
- **Third-Party Type Support**: Extending serialization for types you don't control (like `std::vector`)
- **Non-Intrusive Design**: Types don't need to know about serialization

## Quick Start

```bash
# Build and run the example
make run

# See all available options
make help
```

## Code Structure

```
serialize/
├── serialize.hpp         # CPO definition and built-in serializers
├── test_serialization.cpp # Main example with Person struct
├── debug_test.cpp       # Simple debug/test file
├── Makefile            # Self-contained build system
└── README.md           # This file
```

## Key Concepts Demonstrated

### 1. CPO Definition

The core serialization CPO is defined using TInCuP's standard pattern:

```cpp
namespace tincup::serial {
  inline constexpr struct serialize_ftor final : tincup::cpo_base<serialize_ftor> {
    TINCUP_CPO_TAG("serialize")
    using tincup::cpo_base<serialize_ftor>::operator();
    
    template<typename T>
    requires tincup::invocable_c<serialize_ftor, const T&>
    constexpr auto operator()(const T& value) const
    noexcept(tincup::nothrow_invocable_c<serialize_ftor, const T&>)
    -> tincup::invocable_t<serialize_ftor, const T&> {
      return tag_invoke(*this, value);
    }
  } serialize;
}
```

### 2. User-Defined Type Serialization

Add serialization for your types via `tag_invoke`:

```cpp
namespace my_app {
  struct Person {
    std::string name;
    int age;
  };

  // Provide serialization in the same namespace as your type
  std::string tag_invoke(tincup::serial::serialize_ftor, const Person& p) {
    return json_string::object(
      json_string::key_value("name", tincup::serial::serialize(p.name)) + ", " +
      json_string::key_value("age", tincup::serial::serialize(p.age))
    );
  }
}
```

### 3. Built-in Type Support

Fundamental types get serialization automatically:

```cpp
// In serialize.hpp - built-in serializers for common types
std::string tag_invoke(serialize_ftor, int value);
std::string tag_invoke(serialize_ftor, const std::string& s);
// ... more fundamental types
```

### 4. Third-Party Type Extension

Extend serialization for types you don't control using `cpo_impl`:

```cpp
// In tincup namespace - specialization for std::vector
namespace tincup {
  template<typename T>
  struct cpo_impl<serial::serialize_ftor, std::vector<T>> {
    std::string operator()(const std::vector<T>& vec) const {
      std::vector<std::string> serialized_items;
      for (const auto& item : vec) {
        serialized_items.push_back(serial::serialize(item));
      }
      return json_string::array(serialized_items);
    }
  };
}
```

## Sample Output

```
Serialized Person: {"name": "John Doe", "age": 30}
Serialized Vector: [1, 2, 3, 4, 5]
Serialization tests passed!
```

## Comparison to Networking Example

This serialize example provides the **foundation** that the networking example builds upon:

| Feature | Serialize Example | Networking Example |
|---------|------------------|-------------------|
| **Scope** | Educational basics | Production-ready system |
| **Backends** | Single JSON output | Multiple backends (binary, JSON) |
| **Types** | Simple flat types | Complex nested hierarchies |
| **Extension** | Basic tag_invoke | Full extension patterns |
| **Error Handling** | Minimal | Enhanced diagnostics |
| **Documentation** | Core concepts | Complete usage guide |

The serialize example teaches the fundamental CPO concepts, while the networking example shows how to apply these concepts to build sophisticated, real-world libraries.

## Building

```bash
# Standard build
make

# Run the example  
make run

# Build with debug info
make debug

# Test different C++ standards
make test-cpp20
make test-cpp23

# Clean up
make clean
```

## Integration

This example is completely self-contained and doesn't require CMake. Simply:

1. Ensure you have a C++20 compiler
2. Run `make` to build
3. Run `./test_serialization` to see it work

The example demonstrates the core TInCuP patterns that enable building more sophisticated systems like the networking serialization framework.