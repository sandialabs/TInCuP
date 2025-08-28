# Preferred Implementation Mechanism for TInCuP CPOs

## Problem Statement

The `tag_invoke` pattern, while solving namespace pollution issues inherent in traditional ADL customization, introduces a significant scalability problem in large codebases: **exhaustive ADL search overhead**. 

When multiple libraries in a codebase use `tag_invoke` for unrelated functionality, each CPO invocation must consider *every* `tag_invoke` overload visible through ADL. In a typical scenario:

```cpp
// Library A: Vector math operations
auto tag_invoke(add_assign_ftor, MyVector& target, const MyVector& source);
auto tag_invoke(scale_ftor, MyVector& target, double scalar);

// Library B: Serialization framework  
auto tag_invoke(serialize_ftor, const MyObject& obj, Stream& stream);
auto tag_invoke(deserialize_ftor, MyObject& obj, Stream& stream);

// Library C: Network protocol handlers
auto tag_invoke(handle_request_ftor, const Request& req, Context& ctx);
auto tag_invoke(handle_response_ftor, const Response& resp, Context& ctx);

// Problem: When using vector operations, compiler must consider
// ALL tag_invoke overloads from ALL libraries during overload resolution
MyVector v1, v2;
add_assign(v1, v2);  // Considers 6+ unrelated overloads during compilation
```

This becomes particularly problematic for **specialized domain libraries** like vector frameworks, where:

1. **High CPO Density**: Many operations defined as CPOs (add, subtract, scale, dot product, etc.)
2. **Frequent Usage**: Vector operations called in performance-critical loops 
3. **Template Instantiation**: Generic algorithms create many instantiation points
4. **Compilation Scalability**: Build times degrade with codebase growth

## Proposed Solution: Preferred Implementation Classes

The core idea is to provide an **optional fast-track mechanism** that allows client code to explicitly specify preferred `tag_invoke` implementations, potentially bypassing expensive ADL searches.

### Design Philosophy

- **Opt-in Enhancement**: Existing code continues working unchanged
- **Client Control**: Users choose when to use preferred implementations  
- **Performance Focused**: Primarily targets compile-time optimization
- **Library Isolation**: Enables domain-specific libraries to isolate from unrelated `tag_invoke` overloads
- **Automated Support**: TInCuP code generation can assist with preferred implementation management

## Proposed Implementation Architecture

### 1. Preferred Implementation Classes

Users define classes containing static `tag_invoke` methods for their types:

```cpp
// User-defined preferred implementation class
struct MyVectorPreferredImpl {
    // Vector addition - fast track implementation
    static auto tag_invoke(add_assign_ftor, MyVector& target, const MyVector& source) 
        -> decltype(target.add_assign_impl(source)) {
        return target.add_assign_impl(source);  // Direct method call, no ADL
    }
    
    // Vector scaling - fast track implementation  
    static auto tag_invoke(scale_ftor, MyVector& target, double scalar)
        -> decltype(target.scale_impl(scalar)) {
        return target.scale_impl(scalar);  // Direct method call, no ADL
    }
    
    // Can include multiple CPO implementations for the same types
    static auto tag_invoke(dot_product_ftor, const MyVector& lhs, const MyVector& rhs)
        -> decltype(lhs.dot_impl(rhs)) {
        return lhs.dot_impl(rhs);  // Bypasses ADL entirely
    }
};
```

### 2. Enhanced CPO Dispatch Mechanism

CPOs gain the ability to attempt preferred implementations before falling back to standard ADL:

#### Option A: Template Parameter Approach

```cpp
// Enhanced CPO with preferred implementation support
inline constexpr struct add_assign_ftor final : tincup::cpo_base<add_assign_ftor> {
  TINCUP_CPO_TAG("add_assign")
  
  // Standard dispatch (existing behavior)
  template<typename T, typename U>
    requires tincup::invocable_c<add_assign_ftor, T&, const U&>
  constexpr auto operator()(T& target, const U& source) const
    noexcept(tincup::nothrow_invocable_c<add_assign_ftor, T&, const U&>)
    -> tincup::invocable_t<add_assign_ftor, T&, const U&> {
    return tincup::tag_invoke_cpo(*this, target, source);
  }
  
  // NEW: Preferred implementation dispatch
  template<typename PreferredImpl>
  constexpr auto with_preferred() const {
    return [this](auto&& target, auto&& source) -> decltype(auto) {
      // Try preferred implementation first
      if constexpr (requires { 
        PreferredImpl::tag_invoke(*this, target, source); 
      }) {
        return PreferredImpl::tag_invoke(*this, 
                                        std::forward<decltype(target)>(target),
                                        std::forward<decltype(source)>(source));
      } else {
        // Fallback to standard ADL dispatch
        return tincup::tag_invoke_cpo(*this, 
                                     std::forward<decltype(target)>(target),
                                     std::forward<decltype(source)>(source));
      }
    };
  }
} add_assign;
```

**Usage:**
```cpp
MyVector v1, v2;

// Fast track: bypasses ADL, goes directly to MyVectorPreferredImpl
add_assign.with_preferred<MyVectorPreferredImpl>()(v1, v2);

// Standard: uses normal ADL search (existing behavior)  
add_assign(v1, v2);
```

#### Option B: Preference Token Approach

```cpp
// Preference tokens for cleaner syntax
template<typename PreferredImpl>
struct preference_token {
    using preferred_type = PreferredImpl;
};

template<typename PreferredImpl>
constexpr auto prefer = preference_token<PreferredImpl>{};

// Enhanced CPO with token-based preferred dispatch
inline constexpr struct add_assign_ftor final : tincup::cpo_base<add_assign_ftor> {
  TINCUP_CPO_TAG("add_assign")
  
  // Overload that detects preference token
  template<typename T, typename U, typename PrefToken>
    requires std::is_same_v<PrefToken, preference_token<typename PrefToken::preferred_type>>
  constexpr auto operator()(T& target, const U& source, PrefToken) const {
    using PreferredImpl = typename PrefToken::preferred_type;
    
    if constexpr (requires { PreferredImpl::tag_invoke(*this, target, source); }) {
      return PreferredImpl::tag_invoke(*this, target, source);
    } else {
      return tincup::tag_invoke_cpo(*this, target, source);
    }
  }
  
  // Standard overload (existing behavior)
  template<typename T, typename U>
    requires tincup::invocable_c<add_assign_ftor, T&, const U&>
           && (!std::is_same_v<std::decay_t<U>, preference_token<typename std::decay_t<U>::preferred_type>>)
  constexpr auto operator()(T& target, const U& source) const {
    return tincup::tag_invoke_cpo(*this, target, source);
  }
} add_assign;
```

**Usage:**
```cpp
MyVector v1, v2;

// Fast track with cleaner syntax
add_assign(v1, v2, prefer<MyVectorPreferredImpl>);

// Standard dispatch
add_assign(v1, v2);
```

### 3. TInCuP Code Generation Integration

#### Enhanced CPO Generation

```bash
# Generate CPO with preferred implementation support
cpo-generator '{"cpo_name": "add_assign", "args": ["$T&: target", "$const U&: source"]}' --with-preferred-dispatch
```

**Generated Code:**
```cpp
inline constexpr struct add_assign_ftor final : tincup::cpo_base<add_assign_ftor> {
  TINCUP_CPO_TAG("add_assign")
  
  // Standard dispatch
  template<typename T, typename U>
    requires tincup::invocable_c<add_assign_ftor, T&, const U&>
  constexpr auto operator()(T& target, const U& source) const
    noexcept(tincup::nothrow_invocable_c<add_assign_ftor, T&, const U&>)
    -> tincup::invocable_t<add_assign_ftor, T&, const U&> {
    return tincup::tag_invoke_cpo(*this, target, source);
  }
  
  // Preferred implementation dispatch
  template<typename PreferredImpl>
  constexpr auto with_preferred() const {
    return [this](auto&& target, auto&& source) -> decltype(auto) {
      if constexpr (requires { 
        PreferredImpl::tag_invoke(*this, 
                                std::forward<decltype(target)>(target),
                                std::forward<decltype(source)>(source)); 
      }) {
        return PreferredImpl::tag_invoke(*this, 
                                        std::forward<decltype(target)>(target),
                                        std::forward<decltype(source)>(source));
      } else {
        return tincup::tag_invoke_cpo(*this, 
                                     std::forward<decltype(target)>(target),
                                     std::forward<decltype(source)>(source));
      }
    };
  }
} add_assign;

// Template for user preferred implementations
template<typename VectorType>
struct PreferredVectorOperations {
    // Users can specialize this template for their vector types
    // static auto tag_invoke(add_assign_ftor, VectorType& target, const VectorType& source);
    // static auto tag_invoke(scale_ftor, VectorType& target, double scalar);
    // ... other vector operations
};
```

#### Preferred Implementation Class Generation

```bash
# Scan existing tag_invoke implementations and generate preferred class
cpo-generator scan-implementations --input src/ --types "MyVector,TheirVector" --output preferred_implementations.hpp
```

**Generated Output:**
```cpp
// Auto-generated from existing tag_invoke implementations
struct MyVectorPreferredImpl {
    // Discovered from: auto tag_invoke(add_assign_ftor, MyVector& target, const MyVector& source)
    static auto tag_invoke(add_assign_ftor, MyVector& target, const MyVector& source) {
        return target.add_assign_impl(source);  // Direct implementation call
    }
    
    // Discovered from: auto tag_invoke(scale_ftor, MyVector& target, double scalar)  
    static auto tag_invoke(scale_ftor, MyVector& target, double scalar) {
        return target.scale_impl(scalar);  // Direct implementation call
    }
};

// Convenience alias for users
using MyVectorOps = MyVectorPreferredImpl;
```

## Technical Benefits

### Compile-Time Performance Improvements

1. **Reduced ADL Search Space**: When preferred implementation exists and matches, ADL search is bypassed entirely
2. **Earlier SFINAE Resolution**: Template constraint failures detected faster with direct static method checks
3. **Predictable Instantiation**: More deterministic template instantiation patterns improve compiler caching
4. **Localized Error Messages**: Failures in preferred implementations produce clearer diagnostics

### Quantified Benefits (Theoretical)

```cpp
// Scenario: Large codebase with 50+ tag_invoke overloads from various libraries
// Vector framework defines 20 CPOs, used in 100+ call sites

// Without preferred implementations:
// Each CPO call: O(50) ADL candidates × O(100) call sites = O(5000) overload resolutions

// With preferred implementations (assuming 80% coverage):  
// 80 call sites: O(1) preferred dispatch = O(80) overload resolutions
// 20 call sites: O(50) ADL fallback = O(1000) overload resolutions  
// Total: O(1080) overload resolutions

// Theoretical compile-time improvement: ~80% reduction in overload resolution work
```

### Runtime Performance (Neutral/Positive)

- **No Runtime Overhead**: Preferred implementations compile to identical code as direct method calls
- **Potential Inlining Benefits**: Static methods may inline more aggressively than ADL-dispatched functions
- **Cache Locality**: More predictable call patterns may improve instruction cache behavior

## Use Cases and Adoption Strategy

### Primary Use Case: Domain-Specific Libraries

**Vector/Matrix Math Frameworks:**
```cpp
struct LinearAlgebraOps {
    static auto tag_invoke(add_ftor, Vector& a, const Vector& b) { return a.add_impl(b); }
    static auto tag_invoke(mul_ftor, Matrix& m, const Vector& v) { return m.mul_impl(v); }
    // ... 20+ vector/matrix operations
};

// Used in performance-critical numerical code
Vector result = add.with_preferred<LinearAlgebraOps>()(v1, v2);
```

**Network Protocol Handlers:**
```cpp  
struct ProtocolOps {
    static auto tag_invoke(encode_ftor, const Message& msg, Buffer& buf);
    static auto tag_invoke(decode_ftor, Message& msg, const Buffer& buf);
};
```

**Database ORM Operations:**
```cpp
struct DatabaseOps {
    static auto tag_invoke(save_ftor, Entity& entity, Transaction& txn);
    static auto tag_invoke(load_ftor, Entity& entity, const Key& key);
};
```

### Adoption Levels

1. **Level 1: Manual Preferred Classes**
   - Users manually define preferred implementation classes  
   - Use existing CPO generation, add preferred dispatch manually
   - Immediate benefits for critical performance paths

2. **Level 2: TInCuP-Assisted Generation**
   - TInCuP generates CPOs with preferred dispatch support
   - Tools help create preferred implementation classes
   - Broader adoption across domain-specific libraries

3. **Level 3: Automated Discovery and Maintenance**
   - TInCuP scans codebases and auto-generates preferred classes
   - Maintains synchronization between free functions and preferred implementations
   - Seamless integration for large-scale adoption

## Implementation Challenges and Solutions

### Challenge 1: Template Argument Deduction Complexity

**Problem:** Perfect forwarding through preferred implementation layer while maintaining SFINAE compatibility.

**Solution:** 
```cpp
// Use constexpr if with requires expressions for robust SFINAE
template<typename PreferredImpl>
constexpr auto with_preferred() const {
    return [this]<typename... Args>(Args&&... args) -> decltype(auto) {
        if constexpr (requires { 
            PreferredImpl::tag_invoke(*this, std::forward<Args>(args)...); 
        }) {
            return PreferredImpl::tag_invoke(*this, std::forward<Args>(args)...);
        } else if constexpr (requires { 
            tincup::tag_invoke_cpo(*this, std::forward<Args>(args)...); 
        }) {
            return tincup::tag_invoke_cpo(*this, std::forward<Args>(args)...);
        } else {
            // Trigger enhanced_fail diagnostics
            this->enhanced_fail(std::forward<Args>(args)...);
        }
    };
}
```

### Challenge 2: Error Message Quality

**Problem:** When preferred implementation fails, error messages should be clear about the failure mode.

**Solution:** Enhanced diagnostics that distinguish between preferred and fallback failures:
```cpp
// In enhanced_fail diagnostics
if constexpr (requires { PreferredImpl::tag_invoke(*this, args...); }) {
    static_assert(always_false_v<PreferredImpl>, 
        "Preferred implementation exists but failed to compile. "
        "Check PreferredImpl::tag_invoke signature and constraints.");
} else {
    // Existing enhanced_fail logic for ADL fallback
    ...
}
```

### Challenge 3: API Ergonomics

**Problem:** Balance between performance benefits and syntax complexity.

**Solutions:**
1. **Multiple API Styles**: Support both template parameter and token-based approaches
2. **Convenience Aliases**: `using MyOps = MyVectorPreferredImpl;`
3. **IDE Integration**: TInCuP VSCode/Vim plugins can assist with preferred implementation usage
4. **Documentation**: Clear examples showing when/how to use preferred implementations

### Challenge 4: Maintenance Burden

**Problem:** Keeping preferred implementations synchronized with free function `tag_invoke` overloads.

**Solution:** TInCuP tooling automation:
```bash
# Verify synchronization
cpo-verify --check-preferred-sync --impl-class MyVectorOps --namespace myvector

# Auto-update preferred implementations  
cpo-generator sync-preferred --impl-class MyVectorOps --scan-dir src/
```

## Backwards Compatibility and Migration

### Zero-Impact Introduction

- **Existing Code Unchanged**: All current CPO usage continues working identically
- **Opt-in Feature**: Users choose when/where to use preferred implementations  
- **Gradual Adoption**: Can be introduced incrementally, starting with performance-critical paths

### Migration Strategy

1. **Phase 1: Core Implementation**
   - Add preferred dispatch support to TInCuP CPO base classes
   - Implement template parameter approach first (simpler)
   - Document API and provide examples

2. **Phase 2: Tooling Integration** 
   - Enhance `cpo-generator` to support `--with-preferred-dispatch` flag
   - Add scanning tools to discover existing `tag_invoke` implementations
   - Create preferred implementation class templates

3. **Phase 3: Advanced Features**
   - Implement token-based approach for cleaner syntax
   - Add automated synchronization tools
   - Performance analysis and benchmarking utilities

## Future Enhancements

### Compile-Time Performance Analysis

```bash
# Measure compile-time impact of preferred implementations
cpo-benchmark --measure-compilation --with-preferred --without-preferred \
              --codebase-size large --cpo-density high

# Output:
# Without preferred: 45.2s average compile time
# With preferred:    12.8s average compile time (72% improvement)
# ADL search reduction: 85% of call sites bypass full ADL
```

### Smart Preferred Implementation Selection

```cpp
// Future: Multi-tier preferred implementations
struct FastVectorOps {      // Optimized for speed
    static auto tag_invoke(add_ftor, FastVector& a, const FastVector& b);
};

struct SafeVectorOps {      // Optimized for safety/debugging  
    static auto tag_invoke(add_ftor, SafeVector& a, const SafeVector& b);
};

// Automatic selection based on type traits or build configuration
template<typename VectorType>
using PreferredOpsFor = std::conditional_t<
    is_fast_vector_v<VectorType>, FastVectorOps, SafeVectorOps
>;
```

### Integration with Reflection (Future C++)

When C++ gains reflection capabilities, preferred implementations could be auto-generated:
```cpp
// Hypothetical future C++ with reflection
template<typename T>
auto generate_preferred_impl() {
    // Scan T for tag_invoke-compatible methods
    // Auto-generate preferred implementation class
    // Maintain perfect synchronization
}
```

## Conclusion

The preferred implementation mechanism addresses a fundamental scalability limitation of the `tag_invoke` pattern while maintaining full backwards compatibility. By providing an optional fast-track for `tag_invoke` dispatch, it enables:

- **Significant compile-time performance improvements** for CPO-heavy codebases
- **Library isolation** from unrelated `tag_invoke` implementations  
- **Developer control** over performance-critical code paths
- **Automated tooling support** through TInCuP integration

This enhancement positions TInCuP as not just a code generation tool, but as a comprehensive solution to `tag_invoke` scalability challenges in production codebases. The combination of automated CPO generation with intelligent dispatch optimization creates a compelling value proposition for adopting `tag_invoke`-based customization at scale.

The proposed implementation maintains TInCuP's design philosophy of providing sophisticated tooling to make advanced C++ patterns practical and maintainable, while directly addressing real-world performance concerns that could otherwise limit `tag_invoke` adoption in large systems.