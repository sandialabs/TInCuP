# TInCuP Examples

This directory contains comprehensive examples demonstrating TInCuP's Customization Point Object (CPO) capabilities. The examples progress from basic concepts to production-ready implementations.

## 📚 Learning Path

### 1. **Serialize Example** - Fundamentals
**Location**: `serialize/`  
**Build**: `cd serialize && make run`

Learn the core CPO concepts:
- Basic CPO implementation with `tag_invoke`
- User-defined type serialization
- Third-party type extension via `cpo_impl`
- Non-intrusive design patterns

**Output**:
```
Serialized Person: {"name": "John Doe", "age": 30}
Serialized Vector: [1, 2, 3, 4, 5]
Serialization tests passed!
```

### 2. **Networking Example** - Production System
**Location**: `networking/`  
**Build**: `cd networking && make run`

See CPOs in action for real-world applications:
- Multi-backend serialization (binary + JSON)
- Complex nested type hierarchies
- Enhanced diagnostics and error handling
- Comprehensive extension patterns
- Performance considerations

**Output**:
```
=== TInCuP Networking Serialization Example ===

1. FUNDAMENTAL TYPES
--------------------
Serialized int: 42 (size: 4 bytes)
Serialized string: "Hello, TInCuP!" (total size: 30 bytes)
[... detailed roundtrip testing ...]
```

## 🎯 Key Differences

| Aspect | Serialize Example | Networking Example |
|--------|------------------|-------------------|
| **Purpose** | Educational foundation | Production-ready framework |
| **Complexity** | Simple, focused | Comprehensive, realistic |
| **Backends** | Single JSON format | Multiple backends (binary, JSON) |
| **Types** | Basic structs | Complex nested hierarchies |
| **Error Handling** | Minimal | Enhanced diagnostics |
| **Extension Patterns** | Core `tag_invoke` | Full extension ecosystem |
| **Build System** | Simple Makefile | Complete build + documentation |
| **Use Case** | Learn CPO concepts | Real serialization needs |

## 🚀 Quick Start

```bash
# Learn the basics first
cd serialize
make run

# Then see the full power
cd ../networking  
make run
```

## 🔧 Self-Contained Design

Both examples are **completely self-contained**:
- ✅ No CMake dependency
- ✅ Simple Makefiles with help
- ✅ Use single-include header
- ✅ Comprehensive documentation
- ✅ Easy to copy and modify

## 📖 What You'll Learn

### From the Serialize Example:
- How CPOs provide clean, extensible interfaces
- The `tag_invoke` protocol for type extension
- Using `cpo_impl` for third-party types
- Basic serialization patterns

### From the Networking Example:
- Building production-ready CPO-based libraries
- Multi-format serialization architecture
- Type-safe extension mechanisms
- Enhanced diagnostics and error handling
- Real-world performance considerations

## 🎨 Extending the Examples

Both examples are designed to be **starting points** for your own CPO-based libraries:

- **Serialize**: Modify to support XML, YAML, or custom formats
- **Networking**: Extend with compression, encryption, or protocol-specific features

## 💡 Design Philosophy

These examples demonstrate TInCuP's key strengths:

1. **Clean Interfaces**: Same function call, multiple implementations
2. **Non-Intrusive Extension**: Add functionality without modifying existing code  
3. **Type Safety**: Compile-time verification of operations
4. **Zero-Cost Abstraction**: No runtime overhead for flexibility
5. **Enhanced Diagnostics**: Helpful error messages guide correct usage

The progression from simple to sophisticated shows how CPO-based designs scale from educational examples to production systems while maintaining clean, understandable interfaces.