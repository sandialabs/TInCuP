# TInCuP

<div align="center">
  <img src="tincup.png" alt="TInCuP Logo" width="200"/>
</div>

[![CI](https://github.com/sandialabs/TInCuP/actions/workflows/ci.yml/badge.svg)](https://github.com/sandialabs/TInCuP/actions/workflows/ci.yml)
[![PyPI](https://img.shields.io/pypi/v/TInCuP.svg)](https://pypi.org/project/TInCuP/)
[![Python Versions](https://img.shields.io/pypi/pyversions/TInCuP.svg)](https://pypi.org/project/TInCuP/)
[![CMake](https://img.shields.io/badge/CMake-supported-blue?logo=cmake)](#cmake)
[![Meson](https://img.shields.io/badge/Meson-supported-brightgreen)](#meson)
[![VS%20Code](https://img.shields.io/badge/VS%20Code-integration-007ACC?logo=visualstudiocode)](docs/VSCODE_INTEGRATION.md)
[![Vim](https://img.shields.io/badge/Vim-plugin-019733?logo=vim)](editor_integration/vim/README.md)
[![CLion](https://img.shields.io/badge/CLion-integration-0074D9?logo=jetbrains)](editor_integration/clion/README.md)
[![GCC](https://img.shields.io/badge/GCC-C%2B%2B20-success?logo=gnu)](#supported-compilers)
[![Clang](https://img.shields.io/badge/Clang-C%2B%2B20-success?logo=llvm)](#supported-compilers)
[![MSVC](https://img.shields.io/badge/MSVC-C%2B%2B20-blue?logo=visualstudio)](#supported-compilers)

**TInCuP** (Tag Invoked Customization Points) is a modern, header-only C++20 library that solves the **boilerplate problem** in `tag_invoke`-based customization points through comprehensive code generation and verification tools.

## Installation

- Python tools: `pip install TInCuP==1.0.0` (installs the `cpo-generator` CLI)
- From source (development): `pip install -e .`

## Why Should I Care About Customization Points (and TInCuP?)

Have you ever been in a situation like as a developer like the following?

- You are developing `ImpressiveLib`, a very impressive library.
- `ImpressiveLib` uses `AwesomeType`, which is defined in `CoolLib`, an open source library that you *don't* own. 
- You'd *really* like to use `SpiffyLib`, another library you *don't* own, to do something with `AwesomeType`.
- Unfortunately there is an interface incompatibility. It looks like you need to modify one of the two libraries. (DON'T!) 
- This is where customization points can help!

## The Problem with Existing Approaches

C++ customization mechanisms have fundamental limitations that the standardization committee has recognized in WG21 papers **P1895R0** and **P2279R0**:

- **Namespace Pollution**: Traditional ADL customization requires globally reserving function names
- **Composition Issues**: Multiple libraries can't safely use the same customization point names  
- **Boilerplate Complexity**: The `tag_invoke` pattern, while solving namespace issues, requires significant repetitive code

## TInCuP's Solution

TInCuP bridges the gap between `tag_invoke`'s theoretical benefits and practical usability:

✅ **Eliminates Boilerplate** - Automated code generation from simple JSON specifications  
✅ **Enforces Consistency** - Pattern verification ensures uniform implementations  
✅ **Developer Experience** - Comprehensive IDE integrations make invisible interfaces visible  
✅ **Static Dispatch Integration** - Compile-time optimization for runtime configuration choices  
✅ **Future-Proof** - Standardized patterns enable automated refactoring as the language evolves  

**Bridge Technology**: TInCuP makes `tag_invoke` practical today while C++ evolves toward better built-in solutions.

## Static Dispatch Integration

TInCuP now includes **compile-time dispatch utilities** that enable zero-overhead runtime configuration. These utilities convert runtime decisions into compile-time template specializations, maintaining performance while providing flexible interfaces.

### Boolean Dispatch

Generate CPOs that branch on runtime boolean values with compile-time optimization:

```bash
# Generate a CPO with boolean dispatch
cpo-generator '{"cpo_name": "choose_path", 
                "args": ["$T&: input"], 
                "runtime_dispatch": {
                  "type": "bool", 
                  "dispatch_arg": "selector", 
                  "options": ["yin", "yang"]}}'
```

**Generated CPO:**
```cpp
inline constexpr struct choose_path_ftor final : tincup::cpo_base<choose_path_ftor> {
  TINCUP_CPO_TAG("choose_path")
  
  struct yin_tag {};
  struct yang_tag {};

  // Runtime dispatch with compile-time optimization
  template<typename T>
  constexpr auto operator()(T& input, bool selector = false) const {
    tincup::BoolDispatch dispatcher(selector);
    return dispatcher.receive([&](auto dispatch_constant) {
      if constexpr (dispatch_constant.value) {
        return tag_invoke(*this, input, yin_tag{});
      } else {
        return tag_invoke(*this, input, yang_tag{});
      }
    });
  }

  // Direct compile-time dispatch overloads also available
  template<typename T>
  constexpr auto operator()(T& input, yin_tag) const {
    return tag_invoke(*this, input, yin_tag{});
  }
  
  template<typename T>
  constexpr auto operator()(T& input, yang_tag) const {
    return tag_invoke(*this, input, yang_tag{});
  }
} choose_path;
```

**Usage:**
```cpp
// Both interfaces available for maximum flexibility
auto result1 = choose_path(data, runtime_flag);    // Runtime -> compile-time
auto result2 = choose_path(data, choose_path_ftor::yin_tag{});  // Direct compile-time
```

### String Dispatch

Generate CPOs that dispatch on runtime string values with compile-time specialization:

```bash
# Generate a CPO with string dispatch
cpo-generator '{"cpo_name": "execute_policy", 
                "args": ["$T&: data"], 
                "runtime_dispatch": {
                  "type": "string", 
                  "dispatch_arg": "policy_name", 
                  "options": ["fast", "safe", "debug"]}}'
```

**Generated CPO:**
```cpp
inline constexpr struct execute_policy_ftor final : tincup::cpo_base<execute_policy_ftor> {
  TINCUP_CPO_TAG("execute_policy")
  
  struct fast_tag {};
  struct safe_tag {};
  struct debug_tag {};
  struct not_found_tag {};  // For unrecognized strings

  // Runtime dispatch with string lookup -> compile-time specialization  
  template<typename T>
  constexpr auto operator()(T& data, std::string_view policy_name) const {
    static constexpr std::array<std::string_view, 3> options = {
      "fast", "safe", "debug"
    };
    tincup::StringDispatch<3> dispatcher(policy_name, options);
    return dispatcher.receive([&](auto dispatch_constant) {
      if constexpr (dispatch_constant.value == 0) {
        return tag_invoke(*this, data, fast_tag{});
      } else if constexpr (dispatch_constant.value == 1) {
        return tag_invoke(*this, data, safe_tag{});
      } else if constexpr (dispatch_constant.value == 2) {
        return tag_invoke(*this, data, debug_tag{});
      } else {
        return tag_invoke(*this, data, not_found_tag{});
      }
    });
  }
  
  // Direct compile-time dispatch overloads also available...
} execute_policy;
```

**Usage:**
```cpp
// Runtime string dispatches to compile-time specializations
auto result = execute_policy(data, "fast");  // Zero runtime overhead after dispatch
```

### Performance Benefits

🚀 **Zero Runtime Overhead** - Runtime decisions become compile-time template instantiations  
🚀 **Template Specialization** - Unlock compiler optimizations (loop unrolling, SIMD, constant folding)  
🚀 **Code Generation** - Each dispatch path gets its own optimized implementation  
🚀 **Type Safety** - Maintain compile-time type information through runtime choices  

### When to Use Static Dispatch

✅ **Performance-Critical Code** - Hot paths that benefit from compile-time specialization  
✅ **Configuration-Based Algorithms** - Runtime settings that influence algorithmic choices  
✅ **Hardware Abstraction** - Runtime hardware detection with compile-time optimization  
✅ **Policy-Based Design** - Runtime policy selection with zero-overhead execution  

❌ **Avoid for** - Large option sets (compilation overhead), frequently changing values, simple logic

## Enhanced Error Diagnostics

TInCuP addresses a major limitation identified in **P2279R0**: unhelpful compiler error messages when CPOs are used incorrectly. Our advanced diagnostic system detects common misuse patterns and provides clear, actionable guidance.

### Diagnostic Categories

#### 🔍 **Pointer/Smart Pointer Misuse Detection**
When you accidentally pass a pointer when a reference is expected:

```cpp
std::unique_ptr<Vector> vec_ptr = std::make_unique<Vector>();
add_assign(vec_ptr, other_vec);  // ❌ Passing smart pointer instead of object
```

**Enhanced Error Message**:
```
CPO: No valid tag_invoke overload for these argument types, but there IS a valid 
overload for the dereferenced arguments. Some arguments appear to be pointers/smart_ptrs 
that may need explicit dereferencing. Consider: add_assign(*vec_ptr, other_vec)
```

#### 🔒 **Const-Qualification Misuse Detection** 
When you pass const objects to mutating operations:

```cpp
const Vector vec{1.0, 2.0};
add_assign(vec, other_vec);  // ❌ Passing const object to mutating operation
```

**Enhanced Error Message**:
```
CPO: No valid tag_invoke overload for these argument types, but there IS a valid 
overload for non-const arguments. You may be passing const objects to a mutating operation. 
Consider: add_assign(non_const_vec, other_vec)
```

#### 🔄 **Combined Issue Detection**
When multiple corrections are needed:

```cpp
const std::unique_ptr<Vector> vec_ptr = std::make_unique<Vector>();
add_assign(vec_ptr, other_vec);  // ❌ Both const and pointer issues
```

**Enhanced Error Message**:
```
CPO: No valid tag_invoke overload for these argument types, but there IS a valid 
overload for dereferenced non-const arguments. You may need both pointer dereferencing 
and to remove const-qualification.
```

#### 🔀 **Argument Order Detection**
When binary arguments are swapped:

```cpp
transform(source, target, func);  // ❌ Arguments in wrong order
```

**Enhanced Error Message**:
```
CPO: No valid tag_invoke overload for these argument types, but there IS a valid 
overload with reordered arguments. You may have swapped argument positions. 
Check the expected argument order for this CPO.
```

#### 🔢 **Wrong Argument Count Detection**
When the number of arguments is incorrect:

```cpp
binary_op(single_arg);           // ❌ Missing second argument  
binary_op(arg1, arg2, extra);    // ❌ Too many arguments
```

**Enhanced Error Message**:
```
CPO: No valid tag_invoke overload for this number of arguments, but there IS a valid 
overload with different arity. You may be passing too many or too few arguments. 
Check the expected number of arguments for this CPO.
```

### C++23/C++26 Enhanced Diagnostics

When compiled with C++23, TInCuP provides even more sophisticated error analysis with detailed template instantiation context:

```
CPO [C++23]: No valid tag_invoke for current types, but valid for dereferenced types. 
Problematic arguments need explicit dereferencing. Check template instantiation 
context above for specific argument types.
```

**C++26 User-Generated Static Assert Messages**: When using GCC 14+ or Clang 19+ with `-std=c++2c`, TInCuP takes advantage of P2741R3 user-generated static_assert messages to show the actual CPO name in error messages:

```cpp
// C++26 enhanced error message shows the specific CPO name
static assertion failed: example_cpo
```

This feature works seamlessly with TInCuP's StringLiteral-based CPO registry system, providing much clearer diagnostics by showing exactly which CPO failed rather than generic template error messages.

### Performance Control for Compile-Time Conscious Users

For performance-critical builds, TInCuP provides fine-grained control over diagnostic overhead:

```cpp
// Disable specific diagnostic categories
#define TINCUP_DISABLE_POINTER_DIAGNOSTICS   // Skip pointer/smart_ptr checks
#define TINCUP_DISABLE_CONST_DIAGNOSTICS     // Skip const-qualification checks  
#define TINCUP_DISABLE_ORDER_DIAGNOSTICS     // Skip argument order checks
#define TINCUP_DISABLE_ARITY_DIAGNOSTICS     // Skip argument count checks

// Or disable all enhanced diagnostics at once
#define TINCUP_DISABLE_ALL_DIAGNOSTICS       // Fastest compilation
// or equivalently:
#define TINCUP_MINIMAL_DIAGNOSTICS           // Alias for clarity

// Or use single-knob diagnostic level control
#define TINCUP_DIAGNOSTIC_LEVEL 0   // Disable all diagnostics (fastest)
#define TINCUP_DIAGNOSTIC_LEVEL 1   // Keep pointer/const, disable order/arity
#define TINCUP_DIAGNOSTIC_LEVEL 2   // Keep pointer/const/order, disable arity
#define TINCUP_DIAGNOSTIC_LEVEL 3   // Enable all diagnostics (default)
```

**Compilation Impact**:
- **Default**: 5 additional template checks per failed CPO call
- **Minimal Mode**: Only basic diagnostics (fastest compilation)
- **Selective**: Choose which categories matter most to your codebase

### Technical Benefits

✅ **Educational**: Messages teach correct usage patterns instead of cryptic template errors  
✅ **Version-Aware**: C++23 users automatically get enhanced diagnostics  
✅ **Zero Runtime Cost**: All diagnostics are compile-time only  
✅ **Comprehensive Coverage**: Detects 5 major categories of CPO misuse  
✅ **Performance Conscious**: Opt-out controls for compile-time sensitive builds  
✅ **Bounded Cost**: Maximum 5 additional checks (not factorial explosion)  

---

### References

- [Customization Point Design in C++11 and Beyond](https://ericniebler.com/2014/10/21/customization-point-design-in-c11-and-beyond/)
- [Suggested Design for Customization Points](https://open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4381.html)
- [**P1665r0** Tag based customization points](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p1665r0.pdf)
- [**P1895R0** `tag_invoke`: A general pattern for supporting customisable functions](https://open-std.org/JTC1/SC22/WG21/docs/papers/2019/p1895r0.pdf)
- [Why `tag_invoke` is not the solution I want | Barry's C++ Blog](https://brevzin.github.io/c++/2020/12/01/tag-invoke/)
- [**P2279R0** We need a language mechanism for customization points](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2279r0.html) 
- [**P2547R0** Language support for customisable functions](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2547r0.pdf)
- [**P2855R1** Member customization points for Senders and Receivers](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2855r1.html)


## Features

*   **Header-Only C++20 Library**: A single header `<tincup/tincup.hpp>` provides all necessary components.
*   **Static Dispatch Integration**: Built-in compile-time dispatch utilities (BoolDispatch, StringDispatch) that convert runtime configuration into zero-overhead template specializations.
*   **Enhanced Error Diagnostics**: Comprehensive misuse detection system catching 5 major categories of CPO mistakes with helpful, educational error messages (even more sophisticated with C++23). Performance opt-outs available.
*   **CMake and Meson Support**: First-class integration with both build systems for easy inclusion in any project.
*   **Powerful Code Generator**: A Python-based command-line tool, `cpo-generator`, to automatically generate CPO boilerplate from simple JSON definitions, including static dispatch-enabled CPOs.
*   **IDE/Editor Integration**: Full support for Vim, VSCode, and CLion with plugins, templates, and external tools.
*   **Modern & Friendly**: Designed with modern C++ and Python practices to be user-friendly and easy to extend.

## C++ Library Usage

This is a header-only library. To use it, simply include the main header:

```cpp
#include <tincup/tincup.hpp>
```

## Quick Start

**New to the project?** See `docs/GETTING_STARTED.md` for a complete guide.

### Integration with Build Systems

#### CMake
Standard FetchContent integration works seamlessly:

```cmake
FetchContent_Declare(tincup
  GIT_REPOSITORY https://github.com/sandialabs/TInCuP.git)
FetchContent_MakeAvailable(tincup)
target_link_libraries(your_target PRIVATE tincup::tincup)
```

See `build_systems/cmake/README.md` for advanced usage and `docs/BUILD_SYSTEMS.md` for integration details.

#### Meson
See `build_systems/meson/README.md` for detailed Meson integration.

Quick example:
```meson
tincup_dep = dependency('tincup', fallback : ['tincup', 'tincup_dep'])
executable('my_exe', 'main.cpp', dependencies : [tincup_dep])
```

## Code Generation Tool (`cpo-generator`)

The Python-based code generator is the fastest way to create new CPOs. 

### Installation

Preferred (PyPI):

```bash
pip install TInCuP==1.0.0
```

For development (from source):

```bash
pip install -e .
```

### Generator Syntax

The generator uses a compact JSON syntax to define CPOs. The `args` array takes a list of strings, where each string is a `type: name` pair.

To indicate a template type, prefix the type with a `$` symbol. This will be expanded to `typename` in the generated C++ code.

`"$T&"` becomes `template<typename T> ... (T&)`

#### Argument DSL

Use a compact DSL in the `args` array: each entry is `type: name`. Types can be concrete (e.g., `int`, `double&`) or generic using `$`.

| Token/Pattern      | Meaning                                | Example input              | Generated signature fragment |
|--------------------|----------------------------------------|----------------------------|------------------------------|
| `$T`               | Generic by value                       | `"$T: x"`                 | `template<typename T>(T x)` |
| `$T&`              | Generic lvalue reference                | `"$T&: x"`                | `template<typename T>(T& x)` |
| `$T&&`             | Forwarding reference                    | `"$T&&: x"`               | `template<typename T>(T&& x)` (forwarded) |
| `$T...`            | Generic parameter pack (by value)       | `"$T...: xs"`             | `template<typename... T>(T... xs)` |
| `$T&...`           | Generic lvalue reference pack           | `"$T&...: xs"`            | `template<typename... T>(T&... xs)` |
| `$T&&...`          | Forwarding reference pack               | `"$T&&...: xs"`           | `template<typename... T>(T&&... xs)` (forwarded) |
| `$const T`         | Const-qualified generic                 | `"$const T: x"`           | `template<typename T>(const T x)` |
| `$const T&`        | Const lvalue reference                  | `"$const T&: x"`          | `template<typename T>(const T& x)` |
| `$volatile T&`     | Volatile lvalue reference               | `"$volatile T&: x"`       | `template<typename T>(volatile T& x)` |
| Concrete           | Concrete type (value/ref/rvalue)        | `"int: n"`, `"std::string&&: s"` | `int n`, `std::string&& s` |

Notes:
- Forwarding references are invoked with perfect forwarding, e.g., `std::forward<T>(x)`.
- `const`/`volatile` qualifiers are preserved in parameter types; `volatile` is supported but hasn’t been extensively tested yet.
- Types and parameter names are separated by `:`. Types may include spaces/qualifiers.

### Usage

The tool can be invoked from the command line with a JSON string. Run `cpo-generator --help` for a full list of options and examples.

```bash
# Generate a CPO with Doxygen comments
cpo-generator '{"cpo_name": "my_cpo", "args": ["$V&: x"]}' --doxygen

# Output to a header with include and namespace
cpo-generator '{"cpo_name": "my_cpo", "args": ["$V&: x"]}' \
  --with-include --namespace mylib --out include/mylib/my_cpo.hpp

# Generate a CPO with boolean static dispatch
cpo-generator '{"cpo_name": "choose_path", 
                "args": ["$T&: input"], 
                "runtime_dispatch": {
                  "type": "bool", 
                  "dispatch_arg": "selector", 
                  "options": ["yin", "yang"]}}'

# Generate a CPO with string static dispatch
cpo-generator '{"cpo_name": "execute_policy", 
                "args": ["$T&: data"], 
                "runtime_dispatch": {
                  "type": "string", 
                  "dispatch_arg": "policy_name", 
                  "options": ["fast", "safe", "debug"]}}'

# List LLM-friendly operation patterns
cpo-generator --llm-help
```

### Examples


<details>
<summary><strong>Generic CPO</strong></summary>

A basic CPO with generic template parameters

**Command:**
```bash
cpo-generator {"cpo_name": "generic_cpo", "args": ["$T1&: arg1", "$T2&: arg2"]}
```

**Generated Code:**
```cpp
inline constexpr struct generic_cpo_ftor final : tincup::cpo_base<generic_cpo_ftor> {
  TINCUP_CPO_TAG("generic_cpo")
  inline static constexpr bool is_variadic = false;
    // Typed operator() overload - positive case only (generic)
  // Negative cases handled by tagged fallback in cpo_base
  template<typename T1, typename T2>
    requires tincup::invocable_c<generic_cpo_ftor, T1&, T2&>
  constexpr auto operator()(T1& arg1, T2& arg2) const
    noexcept(tincup::nothrow_invocable_c<generic_cpo_ftor, T1&, T2&>) 
    -> tincup::invocable_t<generic_cpo_ftor, T1&, T2&> {
    return tincup::tag_invoke_cpo(*this, arg1, arg2);
  }
} generic_cpo;

// Note: operator() methods are provided by cpo_base

// CPO-specific concepts and type aliases for convenient usage
template<typename T1, typename T2>
concept generic_cpo_invocable_c = tincup::invocable_c<generic_cpo_ftor, T1&, T2&>;

template<typename T1, typename T2>
concept generic_cpo_nothrow_invocable_c = tincup::nothrow_invocable_c<generic_cpo_ftor, T1&, T2&>;


template<typename T1, typename T2>
using generic_cpo_return_t = tincup::invocable_t<generic_cpo_ftor, T1&, T2&>;

template<typename T1, typename T2>
using generic_cpo_traits = tincup::cpo_traits<generic_cpo_ftor, T1&, T2&>;

// Usage: tincup::is_invocable_v<generic_cpo_ftor, T1&, T2&>
```

</details>

<details>
<summary><strong>Concrete CPO</strong></summary>

A CPO with concrete type parameters

**Command:**
```bash
cpo-generator {"cpo_name": "concrete_cpo", "args": ["int: value", "double&: ref"]}
```

**Generated Code:**
```cpp
[[deprecated("CPO with all concrete types - consider using regular function instead. Intended for experimentation/testing only.")]]
inline constexpr struct concrete_cpo_ftor final : tincup::cpo_base<concrete_cpo_ftor> {
  TINCUP_CPO_TAG("concrete_cpo")
  inline static constexpr bool is_variadic = false;

  // Typed operator() overload - positive case only (concrete)  
  // Negative cases handled by tagged fallback in cpo_base
  constexpr auto operator()(int value, double& ref) const
    noexcept(noexcept(tincup::tag_invoke_cpo(*this, value, ref))) 
    -> decltype(tincup::tag_invoke_cpo(*this, value, ref)) {
    return tincup::tag_invoke_cpo(*this, value, ref);
  }
} concrete_cpo;

// Note: operator() methods are provided by cpo_base

// CPO-specific type aliases for convenient usage (concrete types)
// Note: No concept aliases for concrete types - types are already known
using concrete_cpo_return_t = tincup::invocable_t<concrete_cpo_ftor, int, double&>;
using concrete_cpo_traits = tincup::cpo_traits<concrete_cpo_ftor, int, double&>;

// Usage: tincup::is_invocable_v<concrete_cpo_ftor, int, double&>
```

</details>

<details>
<summary><strong>Forwarding Reference</strong></summary>

A CPO using perfect forwarding for universal references

**Command:**
```bash
cpo-generator {"cpo_name": "forwarding_ref_cpo", "args": ["$T&&: fwd_ref"]}
```

**Generated Code:**
```cpp
inline constexpr struct forwarding_ref_cpo_ftor final : tincup::cpo_base<forwarding_ref_cpo_ftor> {
  TINCUP_CPO_TAG("forwarding_ref_cpo")
  inline static constexpr bool is_variadic = false;
    // Typed operator() overload - positive case only (generic)
  // Negative cases handled by tagged fallback in cpo_base
  template<typename T>
    requires tincup::invocable_c<forwarding_ref_cpo_ftor, T>
  constexpr auto operator()(T&& fwd_ref) const
    noexcept(tincup::nothrow_invocable_c<forwarding_ref_cpo_ftor, T>) 
    -> tincup::invocable_t<forwarding_ref_cpo_ftor, T> {
    return tincup::tag_invoke_cpo(*this, std::forward<T>(fwd_ref)...);
  }
} forwarding_ref_cpo;

// Note: operator() methods are provided by cpo_base

// CPO-specific concepts and type aliases for convenient usage
template<typename T>
concept forwarding_ref_cpo_invocable_c = tincup::invocable_c<forwarding_ref_cpo_ftor, T>;

template<typename T>
concept forwarding_ref_cpo_nothrow_invocable_c = tincup::nothrow_invocable_c<forwarding_ref_cpo_ftor, T>;


template<typename T>
using forwarding_ref_cpo_return_t = tincup::invocable_t<forwarding_ref_cpo_ftor, T>;

template<typename T>
using forwarding_ref_cpo_traits = tincup::cpo_traits<forwarding_ref_cpo_ftor, T>;

// Usage: tincup::is_invocable_v<forwarding_ref_cpo_ftor, T>
```

</details>

<details>
<summary><strong>Variadic Arguments</strong></summary>

A CPO that accepts a variable number of arguments

**Command:**
```bash
cpo-generator {"cpo_name": "variadic_cpo", "args": ["$T&...: variadic_args"]}
```

**Generated Code:**
```cpp
inline constexpr struct variadic_cpo_ftor final : tincup::cpo_base<variadic_cpo_ftor> {
  TINCUP_CPO_TAG("variadic_cpo")
  inline static constexpr bool is_variadic = true;
    // Typed operator() overload - positive case only (generic)
  // Negative cases handled by tagged fallback in cpo_base
  template<typename... T>
    requires tincup::invocable_c<variadic_cpo_ftor, T&...>
  constexpr auto operator()(T&... variadic_args) const
    noexcept(tincup::nothrow_invocable_c<variadic_cpo_ftor, T&...>) 
    -> tincup::invocable_t<variadic_cpo_ftor, T&...> {
    return tincup::tag_invoke_cpo(*this, variadic_args...);
  }
} variadic_cpo;

// Note: operator() methods are provided by cpo_base

// CPO-specific concepts and type aliases for convenient usage
template<typename... T>
concept variadic_cpo_invocable_c = tincup::invocable_c<variadic_cpo_ftor, T&...>;

template<typename... T>
concept variadic_cpo_nothrow_invocable_c = tincup::nothrow_invocable_c<variadic_cpo_ftor, T&...>;


template<typename... T>
using variadic_cpo_return_t = tincup::invocable_t<variadic_cpo_ftor, T&...>;

template<typename... T>
using variadic_cpo_traits = tincup::cpo_traits<variadic_cpo_ftor, T&...>;

// Usage: tincup::is_invocable_v<variadic_cpo_ftor, T&...>
```

</details>

<details>
<summary><strong>Boolean Dispatch CPO</strong></summary>

A CPO with runtime boolean dispatch that compiles to zero-overhead compile-time specialization

**Command:**
```bash
cpo-generator {"cpo_name": "conditional_process", "args": ["$T&: data"], "runtime_dispatch": {"type": "bool", "dispatch_arg": "use_fast_path", "options": ["fast", "safe"]}}
```

**Generated Code:**
```cpp
inline constexpr struct conditional_process_ftor final : tincup::cpo_base<conditional_process_ftor> {
  TINCUP_CPO_TAG("conditional_process")
  inline static constexpr bool is_variadic = false;  
  static constexpr struct fast_tag {} fast;
  static constexpr struct safe_tag {} safe;

// Runtime dispatch overload
template<typename T>
constexpr auto operator()(T& data, bool use_fast_path = false) const
  requires (tincup::invocable_c<conditional_process_ftor, T&, fast_tag> && \
            tincup::invocable_c<conditional_process_ftor, T&, safe_tag>)
{
  tincup::BoolDispatch dispatcher(use_fast_path);
  return dispatcher.receive([&](auto dispatch_constant) {
    if constexpr (dispatch_constant.value) {
      return tag_invoke(*this, data, fast);
    } else {
      return tag_invoke(*this, data, safe);
    }
  });
}

// Compile-time dispatch overload for fast
template<typename T>
constexpr auto operator()(T& data, fast_tag) const
  requires (tincup::invocable_c<conditional_process_ftor, T&, fast_tag>)
{
  return tag_invoke(*this, data, fast);
}

// Compile-time dispatch overload for safe
template<typename T>
constexpr auto operator()(T& data, safe_tag) const
  requires (tincup::invocable_c<conditional_process_ftor, T&, safe_tag>)
{
  return tag_invoke(*this, data, safe);
}

} conditional_process;

// Note: operator() methods are provided by cpo_base

// CPO-specific concepts and type aliases for convenient usage
template<typename T>
concept conditional_process_invocable_c = tincup::invocable_c<conditional_process_ftor, T&>;

template<typename T>
concept conditional_process_nothrow_invocable_c = tincup::nothrow_invocable_c<conditional_process_ftor, T&>;


template<typename T>
using conditional_process_return_t = tincup::invocable_t<conditional_process_ftor, T&>;

template<typename T>
using conditional_process_traits = tincup::cpo_traits<conditional_process_ftor, T&>;

// Usage: tincup::is_invocable_v<conditional_process_ftor, T&>
```

**Usage:**
```cpp
// Runtime usage with compile-time optimization
auto result = conditional_process(data, runtime_flag);

// Direct compile-time usage
auto result = conditional_process(data, conditional_process_ftor::fast{});
```

</details>

<details>
<summary><strong>String Dispatch CPO</strong></summary>

A CPO with runtime string dispatch that converts string selection into compile-time specialization

**Command:**
```bash
cpo-generator {"cpo_name": "compression_method", "args": ["$const T&: input"], "runtime_dispatch": {"type": "string", "dispatch_arg": "algorithm", "options": ["lz4", "zstd", "gzip"]}}
```

**Generated Code:**
```cpp
inline constexpr struct compression_method_ftor final : tincup::cpo_base<compression_method_ftor> {
  TINCUP_CPO_TAG("compression_method")
  inline static constexpr bool is_variadic = false;   
  static constexpr struct lz4_tag {} lz4; 
  static constexpr struct zstd_tag {} zstd; 
  static constexpr struct gzip_tag {} gzip;
  static constexpr struct not_found_tag {} not_found;

  // Precomputed options table as a class-scope constant (single instance across TUs)
  inline static constexpr auto options_array = tincup::string_view_array<3>{     "lz4",     "zstd",     "gzip"  };

  // Runtime dispatch overload for string
  template<typename T>
  requires tincup::invocable_c<compression_method_ftor, const T&, compression_method_ftor::not_found_tag>
  constexpr auto operator()(const T& input, std::string_view algorithm) const 
  noexcept(tincup::nothrow_invocable_c<compression_method_ftor, const T&, compression_method_ftor::not_found_tag>) {
    tincup::StringDispatch<3> dispatcher(algorithm, options_array);
    return dispatcher.receive([&](auto dispatch_constant) {
      if constexpr (dispatch_constant.value < 3) {
        if constexpr (dispatch_constant.value == 0) {
          return tag_invoke(*this, input, lz4);
        }
        if constexpr (dispatch_constant.value == 1) {
          return tag_invoke(*this, input, zstd);
        }
        if constexpr (dispatch_constant.value == 2) {
          return tag_invoke(*this, input, gzip);
        }
      } else {
        return tag_invoke(*this, input, not_found);
      }
    });
  }

  // Compile-time dispatch overloads for string
  template<typename T>
  requires tincup::invocable_c<compression_method_ftor, const T&, lz4_tag>
  constexpr auto operator()(const T& input, lz4_tag) const
  noexcept(tincup::nothrow_invocable_c<compression_method_ftor, const T&, lz4_tag>) {
    return tag_invoke(*this, input, lz4);
  }
  template<typename T>
  requires tincup::invocable_c<compression_method_ftor, const T&, zstd_tag>
  constexpr auto operator()(const T& input, zstd_tag) const
  noexcept(tincup::nothrow_invocable_c<compression_method_ftor, const T&, zstd_tag>) {
    return tag_invoke(*this, input, zstd);
  }
  template<typename T>
  requires tincup::invocable_c<compression_method_ftor, const T&, gzip_tag>
  constexpr auto operator()(const T& input, gzip_tag) const
  noexcept(tincup::nothrow_invocable_c<compression_method_ftor, const T&, gzip_tag>) {
    return tag_invoke(*this, input, gzip);
  }
  
  template<typename T>
  requires tincup::invocable_c<compression_method_ftor, const T&, not_found_tag>
  constexpr auto operator()(const T& input, not_found_tag) const
  noexcept(tincup::nothrow_invocable_c<compression_method_ftor, const T&, not_found_tag>) {
    return tag_invoke(*this, input, not_found);
  }

} compression_method;

// Note: operator() methods are provided by cpo_base

// CPO-specific concepts and type aliases for convenient usage
template<typename T>
concept compression_method_invocable_c = tincup::invocable_c<compression_method_ftor, const T&>;

template<typename T>
concept compression_method_nothrow_invocable_c = tincup::nothrow_invocable_c<compression_method_ftor, const T&>;


template<typename T>
using compression_method_return_t = tincup::invocable_t<compression_method_ftor, const T&>;

template<typename T>
using compression_method_traits = tincup::cpo_traits<compression_method_ftor, const T&>;

// Usage: tincup::is_invocable_v<compression_method_ftor, const T&>
```

**Usage:**
```cpp
// Runtime string -> compile-time dispatch (zero overhead after lookup)
auto compressed = compression_method(data, "lz4");

// Direct compile-time usage
auto compressed = compression_method(data, compression_method_ftor::lz4_tag{});
```

</details>


**Static Dispatch Benefits:**
- 🚀 Zero runtime overhead after initial dispatch
- 🎯 Compiler can fully optimize each path independently  
- 🔧 Both runtime convenience and compile-time performance
- 🛡️ Type-safe dispatch with comprehensive error checking

## All-Concrete CPO Design Guidance

TInCuP provides clear guidance when CPOs may not be the right design choice. **All-concrete CPOs** (those with only concrete types, no generic parameters) are marked with `[[deprecated]]` to signal they should typically be regular functions instead:

```cpp
// Generated all-concrete CPO - includes deprecation warning
[[deprecated("CPO with all concrete types - consider using regular function instead. Intended for experimentation/testing only.")]]
inline constexpr struct concrete_example_ftor final : tincup::cpo_base<concrete_example_ftor> {
  TINCUP_CPO_TAG("concrete_example")
  
  // Pure concrete implementation - no concepts needed
  constexpr auto operator()(int& value, const std::string& name) const
    noexcept(noexcept(tincup::tag_invoke_cpo(*this, value, name)))
    -> decltype(tincup::tag_invoke_cpo(*this, value, name)) {
    return tincup::tag_invoke_cpo(*this, value, name);
  }
} concrete_example;
```

### When All-Concrete CPOs Make Sense

✅ **Library Consistency**: All operations use CPO pattern for uniform API design  
✅ **Experimentation**: Testing CPO patterns before genericizing  
✅ **Gradual Migration**: Converting regular functions to CPOs incrementally  
✅ **Future Extension**: Planning to add generic support later  

### When Regular Functions Are Better

❌ **Single Implementation**: Only one implementation will ever exist  
❌ **No Customization**: No third-party customization needed  
❌ **Simple Logic**: Straightforward operations without extension points  
❌ **Performance Critical**: Avoiding any CPO overhead  

### Technical Implementation

All-concrete CPOs receive special treatment in code generation:

- **No Concept Predicates**: Use direct `decltype()` and `noexcept()` evaluation instead of concept-based checks
- **Proper ADL**: Use fully-qualified `tincup::tag_invoke_cpo()` for reliable argument-dependent lookup
- **Compile-Time Correctness**: Direct type evaluation without template indirection
- **Clear Deprecation Message**: Compiler warning explains design implications

**Example Compiler Warning:**
```cpp
int value = 42;
std::string name = "test";
auto result = concrete_example(value, name);  // Warning: deprecated CPO usage
```

```
warning: 'concrete_example' is deprecated: CPO with all concrete types - 
consider using regular function instead. Intended for experimentation/testing only.
```

This guidance helps developers make informed architectural decisions while still supporting legitimate use cases for all-concrete CPOs.

## Limitations

### Code Generator Limitations

*   **No Template-Template Parameters**: The generator does not support template-template parameters.
*   **No Non-Type Template Parameters**: The generator does not support non-type template parameters (e.g., `template<int N>`).

### C++20 Language Constraints

TInCuP addresses most issues raised in **P2279R0 "We need a language mechanism for customization points"** within the bounds of what's possible in C++20. However, some desired features require future language evolution:

#### ✅ **Fully Addressed by TInCuP**
- **Poor Error Messages**: TInCuP's diagnostic system transforms cryptic template errors into educational guidance
- **Boilerplate Complexity**: Comprehensive code generation eliminates repetitive `tag_invoke` patterns
- **Developer Experience**: IDE integrations, introspection, and verification tools make CPOs practical

#### 🔶 **Partially Addressed**  
- **Composition**: TInCuP provides `compose(f, g, ...)` utilities, but not seamless language-level composition
- **Discoverability**: Auto-generated documentation helps, but requires explicit registration rather than automatic discovery

#### ❌ **Requires Future Language Support**
- **Syntactic Sugar**: P2279R0's desired `customizable void f(int x);` syntax needs language changes
- **Automatic ADL Isolation**: Built-in namespace isolation requires compiler support  
- **Seamless Composition**: True language-level composition awaits reflection/meta-programming features

**Assessment**: TInCuP represents the optimal C++20 solution to customization point challenges. While better language mechanisms are eventually needed, TInCuP demonstrates that current pain points can be dramatically reduced through sophisticated tooling and library design.


## Third-Party Types

When you need to customize CPOs for types you do not control (for example, `Kokkos::View<...>`), TInCuP provides a formatter-style extension point that avoids adding symbols to third-party namespaces and requires no wrappers at call sites.

- Extension point: specialize `tincup::cpo_impl<CPO, T>` in your project and implement `static auto call(T&, Args&&...)`.
- Discovery: This trait mirrors the approach used by `std::formatter`. Specializations live in namespace `tincup` and do not require modifying third-party headers.
- Generation support: use the generator to emit a skeleton specialization.

Example: generate a trait specialization for a string-dispatch CPO targeting a third-party template type

```bash
cpo-generator '{"cpo_name":"execute_policy", "args":["$T&: data"], "runtime_dispatch":{"type":"string","dispatch_arg":"policy","options":["fast","safe","debug"]}}'   --emit-trait-impl --impl-target 'Kokkos::View<...>' --out include/myproj/execute_policy_impl.hpp
```

The generated skeleton looks like this (simplified):

```cpp
namespace tincup {
template<class... P>
struct cpo_impl<execute_policy_ftor, Kokkos::View<P...>> {
  template<class... Args>
  static auto call(Kokkos::View<P...>& view, Args&&... args)
      -> /* return type */ {
    // TODO: implement using 'view' and (args...)
  }
};
} // namespace tincup
```

Notes:
- You can wrap emission in a macro guard with `--impl-guard MACRO`.
- This approach avoids defining functions in third-party namespaces and does not require inheritance or wrappers at call sites.
- It is suitable for both concrete and templated third-party types; use `'...'` in `--impl-target` to denote a template parameter pack (e.g., `Kokkos::View<...>`).

## Vim Plugin

This repository contains a fully functional Vim plugin that integrates the code generator.

### Installation

Using a plugin manager like [vim-plug](https://github.com/junegunn/vim-plug), add the following to your `.vimrc`:

```vim
Plug 'sandialabs/TInCuP', {'rtp': 'editor_integration/vim'}
```

### Usage

The plugin provides two commands:

*   `:CPO <name> [args...]`: Generates a CPO.
*   `:CPOD <name> [args...]`: Generates a CPO with a Doxygen comment stub.

Example:

```vim
:CPO my_cpo $V&:x $const S&:y
```

## Project Structure

```
tincup/
├── build_systems/             # Integration with build systems
│   ├── cmake/                 # CMake integration
│   ├── make/                  # Development tasks (Makefile)
│   └── meson/                 # Meson integration
├── cpo_tools/                 # Code generation and verification tools
│   └── templates/             # Jinja2 templates for C++ generation
├── docs/                      # Documentation and guides
├── editor_integration/        # Editor/IDE support
│   ├── clion/                 # CLion external tools and templates
│   │   └── file_templates/    # CLion file template definitions
│   ├── vim/                   # Complete Vim plugin
│   │   ├── autoload/          # Vim autoload functions
│   │   ├── cpp_cpo/           # CPO-specific functionality
│   │   └── plugin/            # Vim plugin entry points
│   └── vscode/                # VSCode tasks, snippets, configs
├── examples                   # Demonstration of TInCuP in practice
│   └── serialize
├── include/                   # Header-only library
│   └── tincup/                # Main (only) library header
├── scripts                    # Utilities for updating README examples and copyright banner
└── tests/                     # Comprehensive test suite
```

For detailed guidance on any integration, see the README in the respective directory.

## Editor/IDE Integration

### VSCode
See `docs/VSCODE_INTEGRATION.md` for tasks, snippets, and integration tips.

### Vim
The repository includes a complete Vim plugin. See `editor_integration/vim/README.md` for installation and usage.

### CLion
Full CLion integration with external tools, live templates, and file templates. See `editor_integration/clion/README.md` for setup and usage.

## LLM Mode

Use semantic patterns via `operation_type` to simplify generation. Run `cpo-generator --llm-help` to list patterns. Current patterns include:

- mutating_binary: modifies first using second, returns void
- scalar_mutating: modifies object using a scalar, returns void
- unary_mutating: modifies object using a unary function, returns void
- binary_query: computes value from two objects, returns value
- unary_query: computes value from one object, returns value
- generator: creates new object from existing, returns new object
- binary_transform: transforms target using source and a function, returns void

### Operation Patterns

| Operation Type     | Description                                      | Argument Shape                                      |
|--------------------|--------------------------------------------------|-----------------------------------------------------|
| `mutating_binary`  | Modifies first object using second               | `"$T&: target", "$const U&: source"`               |
| `scalar_mutating`  | Modifies object using a scalar value             | `"$T&: target", "$S: scalar"`                      |
| `unary_mutating`   | Modifies object using a unary function           | `"$T&: target", "$F: func"`                        |
| `binary_query`     | Computes value from two objects                  | `"$const T&: lhs", "$const U&: rhs"`               |
| `unary_query`      | Computes value from one object                   | `"$const T&: obj"`                                  |
| `generator`        | Creates new object from existing object          | `"$const T&: source"`                               |
| `binary_transform` | Transforms target using source and a function    | `"$T&: target", "$const U&: source", "$F: func"`   |

Example:

```bash
cpo-generator '{"cpo_name": "add_in_place", "operation_type": "mutating_binary"}' --doxygen
```

## Composition

Compose CPOs (or any invocables) left-to-right using `tincup::compose`:

```cpp
#include <tincup/tincup.hpp>
using namespace tincup;

// Suppose normalize(x) -> y and stringify(y) -> std::string
inline constexpr struct normalize_ftor final : cpo_base<normalize_ftor> {
  TINCUP_CPO_TAG("normalize")
  template<typename T>
    requires invocable_c<normalize_ftor, T>
    constexpr auto operator()(T&& x) const
    noexcept(nothrow_invocable_c<normalize_ftor, T>) -> invocable_t<normalize_ftor, T> {
      return tag_invoke(*this, std::forward<T>(x));
    }
  template<typename T>
    requires (!invocable_c<normalize_ftor, T>)
    constexpr void operator()(T&& x) const { this->enhanced_fail(std::forward<T>(x)); }
} normalize;

inline constexpr struct stringify_ftor final : cpo_base<stringify_ftor> {
  TINCUP_CPO_TAG("stringify")
  template<typename U>
    requires invocable_c<stringify_ftor, U>
    constexpr auto operator()(U&& u) const
    noexcept(nothrow_invocable_c<stringify_ftor, U>) -> invocable_t<stringify_ftor, U> {
      return tag_invoke(*this, std::forward<U>(u));
    }
  template<typename U>
    requires (!invocable_c<stringify_ftor, U>)
    constexpr void operator()(U&& u) const { this->enhanced_fail(std::forward<U>(u)); }
} stringify;

// Compose: stringify(normalize(x))
auto to_string = compose(normalize, stringify);
// Usage
// std::string s = to_string(obj);
```

## CPO Introspection

TInCuP provides comprehensive introspection capabilities for CPOs through `cpo_traits`:

```cpp
#include <tincup/tincup.hpp>
using namespace tincup;

// Example CPO for introspection
struct my_cpo_ftor final : cpo_base<my_cpo_ftor> {
    template<typename T, typename U>
        requires invocable_c<my_cpo_ftor, T&, const U&>
    constexpr auto operator()(T& target, const U& source) const
        noexcept(nothrow_invocable_c<my_cpo_ftor, T&, const U&>)
        -> invocable_t<my_cpo_ftor, T&, const U&> {
        return tag_invoke(*this, target, source);
    }
    
    template<typename T, typename U>
        requires (!invocable_c<my_cpo_ftor, T&, const U&>)
    constexpr void operator()(T& target, const U& source) const {
        this->enhanced_fail(target, source);
    }
} my_cpo;

// Introspect the CPO
using traits = cpo_traits<my_cpo_ftor, std::string&, const int&>;

static_assert(traits::arity == 2);                    // Number of arguments
static_assert(traits::invocable);                     // Can be invoked with these types
static_assert(traits::nothrow_invocable);             // Is noexcept with these types
static_assert(traits::is_void_returning);             // Returns void
static_assert(traits::all_args_are_refs);             // All arguments are references
static_assert(!traits::all_args_are_const_refs);      // Not all are const references

// Access individual argument types
using first_arg = traits::arg_t<0>;   // std::string&
using second_arg = traits::arg_t<1>;  // const int&

// Get a hint about the signature for debugging
constexpr auto sig = traits::signature_hint();  // Returns "(T, U)"
```

### Available Introspection Features

- **`invocable`**: Whether the CPO can be invoked with given argument types
- **`nothrow_invocable`**: Whether the CPO is noexcept with given argument types  
- **`arity`**: Number of arguments
- **`return_t`**: Return type (void if not invocable)
- **`is_void_returning`**: Whether the return type is void
- **`is_const_invocable`**: Whether the CPO itself is const-qualified
- **`all_args_are_refs`**: Whether all arguments are reference types
- **`all_args_are_const_refs`**: Whether all arguments are const references
- **`arg_t<I>`**: Type of the I-th argument
- **`signature_hint()`**: String representation for debugging

## Supported Compilers

TInCuP is tested on CI with strict C++20 compliance:

- **GCC**: 12+ (tested on Ubuntu, comprehensive C++20 support)
- **Clang**: 15+ (tested on Ubuntu/macOS, excellent C++20 support)  
- **MSVC**: 2019 16.11+ / 2022+ (tested on Windows, `/std:c++20 /permissive-`)

All compilers are tested with:
- Full C++20 feature set (concepts, consteval, etc.)
- Template metaprogramming intensive code
- Strict warning levels (`-Wall -Wextra` / `/W4`)
- Warnings treated as errors for quality assurance

### MSVC-Specific Notes
- Requires `/std:c++20 /permissive-` flags
- Tested with both Debug (`/Od`) and Release (`/O2`) optimizations
- Compatible with Visual Studio 2019 16.11+ and Visual Studio 2022
- Full support for C++20 concepts, consteval, and template features

## Vim Installer (Optional)

To help with setup, a helper script prints or appends the correct vimrc snippet for your plugin manager (vim-plug, Vundle, or Pathogen):

```bash
# Print recommended lines based on your ~/.vimrc
editor_integration/vim/install.sh

# Append to ~/.vimrc with a timestamped backup
editor_integration/vim/install.sh --apply

# Force a specific manager
editor_integration/vim/install.sh --manager plug   # or vundle | pathogen
```

Notes:
- For vim-plug, the script uses `{'rtp': 'editor_integration/vim'}`.
- For Vundle/Pathogen, it adds a `set rtp+=.../editor_integration/vim` line so Vim sees the plugin inside the repo.
- Without a plugin manager, use native packages: symlink `editor_integration/vim/{plugin,autoload}` into `~/.vim/pack/tincup/start/tincup/`.

## Local Testing (Mirrors CI Exactly)

**🚀 Test locally before pushing to avoid CI failures:**

```bash
# Run complete local CI suite (mirrors GitHub Actions exactly)
./run_local_ci.sh

# Quick development testing (faster subset)
./run_local_ci.sh --quick

# Individual test categories  
make test-cmake     # Test CMake build
make test-meson     # Test Meson build
make test-python    # Python tests
make test-examples  # Example compilation
```

**Why local testing matters:**
- ✅ **2-5 minute feedback** vs 10-15 minutes in CI
- ✅ **Exact CI parity** - same commands, same results  
- ✅ **Catch issues early** - before broken CI commits
- ✅ **Debug locally** - reproduce CI failures instantly

See **[docs/LOCAL_TESTING.md](docs/LOCAL_TESTING.md)** for complete guide.

## Contributing

1. **Development setup**: `make -f build_systems/make/Makefile install-dev`
2. **Test locally**: `./run_local_ci.sh` ← **Run this before every push**
3. **Run Python tests**: `make -f build_systems/make/Makefile test`  
4. **Verify patterns**: `make -f build_systems/make/Makefile verify-cpos`

## License

This project is licensed under the MIT License. See the `LICENSE` file for details.
