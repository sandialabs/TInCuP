# Code Generation Tool (`cpo-generator`)

The Python-based code generator is the fastest way to create new CPOs.

## Installation

Preferred (PyPI):

```bash
# latest
pip install TInCuP

# exact version (this release)
pip install TInCuP==1.0.4
```

For development (from source):

```bash
pip install -e .
```

## Generator Syntax

The generator uses a compact JSON syntax to define CPOs. The `args` array takes a list of strings, where each string is a `type: name` pair.

To indicate a template type, prefix the type with a `$` symbol. This will be expanded to `typename` in the generated C++ code.

`"$T&"` becomes `template<typename T> ... (T&)`

### Argument DSL

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
inline constexpr struct generic_cpo_ftor final :tincup::cpo_base<generic_cpo_ftor> {
  TINCUP_CPO_TAG("generic_cpo")
  inline static constexpr bool is_variadic = false;
  // Re-expose base operator() so failures route to diagnostics
  using tincup::cpo_base<generic_cpo_ftor>::operator();
    // Typed operator() overload - positive case only (generic)
  // Negative cases handled by tagged fallback in cpo_base
  template<typename T1, typename T2>
    requires tincup::invocable_c<generic_cpo_ftor, T1&, T2&>
  constexpr auto operator()(T1& arg1, T2& arg2) const
    noexcept(tincup::nothrow_invocable_c<generic_cpo_ftor, T1&, T2&>)
    -> tincup::invocable_t<generic_cpo_ftor, T1&, T2&> {
    return tag_invoke(*this, arg1, arg2);
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

// Usage:tincup::is_invocable_v<generic_cpo_ftor, T1&, T2&>


// External generator-provided argument trait specialization for generic_cpo
// Forward declare primary template to allow specialization even if not included yet
namespace tincup { template<typename Cp, typename...Args> struct cpo_arg_traits; }

namespace tincup {
  template<typename T1, typename T2>
  struct cpo_arg_traits<generic_cpo_ftor, T1&, T2&> {
    static constexpr bool available = true;
    // Fixed (non-pack) argument count
    static constexpr std::size_t fixed_arity = 2;

    // Helpers to build repeated masks for parameter packs
    static constexpr tincup::arity_type repeat_mask(std::size_t offset, std::size_t count) {
      tincup::arity_type m = tincup::arity_type{0};
      for (std::size_t i = 0; i < count; ++i) m |= (tincup::arity_type{1} << (offset + i));
      return m;
    }

    // Values mask
    static constexpr tincup::arity_type values_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      // Fixed positions
      // Pack positions
      return m;
    }() ;

    // Pointers mask
    static constexpr tincup::arity_type pointers_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Lvalue refs mask
    static constexpr tincup::arity_type lvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
 m |= (tincup::arity_type{1} << 0);  m |= (tincup::arity_type{1} << 1);       return m;
    }() ;

    // Rvalue refs mask (non-forwarding)
    static constexpr tincup::arity_type rvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Forwarding refs mask
    static constexpr tincup::arity_type forwarding_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Lvalue const refs mask
    static constexpr tincup::arity_type lvalue_const_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Const-qualified mask (applies to values, refs, or pointers where declared const)
    static constexpr tincup::arity_type const_qualified_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;
  };
}
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
inline constexpr struct concrete_cpo_ftor final :tincup::cpo_base<concrete_cpo_ftor> {
  TINCUP_CPO_TAG("concrete_cpo")
  inline static constexpr bool is_variadic = false;
  // Re-expose base operator() so failures route to diagnostics
  using tincup::cpo_base<concrete_cpo_ftor>::operator();

  // Typed operator() overload - positive case only (concrete)  
  // Negative cases handled by tagged fallback in cpo_base
  constexpr auto operator()(int value, double& ref) const
    noexcept(noexcept(tag_invoke(*this, value, ref))) 
    -> decltype(tag_invoke(*this, value, ref)) {
    return tag_invoke(*this, value, ref);
  }
} concrete_cpo;

// Note: operator() methods are provided by cpo_base

// CPO-specific type aliases for convenient usage (concrete types)
// Note: No concept aliases for concrete types - types are already known
using concrete_cpo_return_t = tincup::invocable_t<concrete_cpo_ftor, int, double&>;
using concrete_cpo_traits = tincup::cpo_traits<concrete_cpo_ftor, int, double&>;

// Usage:tincup::is_invocable_v<concrete_cpo_ftor, int, double&>

// External generator-provided argument trait specialization for concrete_cpo (concrete)
// Forward declare primary template to allow specialization even if not included yet
namespace tincup { template<typename Cp, typename...Args> struct cpo_arg_traits; }

namespace tincup {
  template<typename _A0, typename _A1>
  struct cpo_arg_traits<concrete_cpo_ftor,_A0, _A1> {
    static constexpr bool available = true;
    static constexpr std::size_t fixed_arity = 2;
    static constexpr tincup::arity_type values_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
        m |= (tincup::arity_type{1} << 0);
      return m;
    }() ;
    static constexpr tincup::arity_type pointers_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;
    static constexpr tincup::arity_type lvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
 m |= (tincup::arity_type{1} << 1);       return m;
    }() ;
    static constexpr tincup::arity_type rvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;
    static constexpr tincup::arity_type forwarding_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;
    static constexpr tincup::arity_type lvalue_const_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;
    static constexpr tincup::arity_type const_qualified_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;
  };
}
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
inline constexpr struct forwarding_ref_cpo_ftor final :tincup::cpo_base<forwarding_ref_cpo_ftor> {
  TINCUP_CPO_TAG("forwarding_ref_cpo")
  inline static constexpr bool is_variadic = false;
  // Re-expose base operator() so failures route to diagnostics
  using tincup::cpo_base<forwarding_ref_cpo_ftor>::operator();
    // Typed operator() overload - positive case only (generic)
  // Negative cases handled by tagged fallback in cpo_base
  template<typename T>
    requires tincup::invocable_c<forwarding_ref_cpo_ftor, T>
  constexpr auto operator()(T&& fwd_ref) const
    noexcept(tincup::nothrow_invocable_c<forwarding_ref_cpo_ftor, T>) 
    -> tincup::invocable_t<forwarding_ref_cpo_ftor, T> {
    return tag_invoke(*this, std::forward<T>(fwd_ref));
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

// Usage:tincup::is_invocable_v<forwarding_ref_cpo_ftor, T>


// External generator-provided argument trait specialization for forwarding_ref_cpo
// Forward declare primary template to allow specialization even if not included yet
namespace tincup { template<typename Cp, typename...Args> struct cpo_arg_traits; }

namespace tincup {
  template<typename T>
  struct cpo_arg_traits<forwarding_ref_cpo_ftor, T> {
    static constexpr bool available = true;
    // Fixed (non-pack) argument count
    static constexpr std::size_t fixed_arity = 1;

    // Helpers to build repeated masks for parameter packs
    static constexpr tincup::arity_type repeat_mask(std::size_t offset, std::size_t count) {
      tincup::arity_type m = tincup::arity_type{0};
      for (std::size_t i = 0; i < count; ++i) m |= (tincup::arity_type{1} << (offset + i));
      return m;
    }

    // Values mask
    static constexpr tincup::arity_type values_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      // Fixed positions
      // Pack positions
      return m;
    }() ;

    // Pointers mask
    static constexpr tincup::arity_type pointers_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Lvalue refs mask
    static constexpr tincup::arity_type lvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Rvalue refs mask (non-forwarding)
    static constexpr tincup::arity_type rvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
 m |= (tincup::arity_type{1} << 0);       return m;
    }() ;

    // Forwarding refs mask
    static constexpr tincup::arity_type forwarding_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
 m |= (tincup::arity_type{1} << 0);       return m;
    }() ;

    // Lvalue const refs mask
    static constexpr tincup::arity_type lvalue_const_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Const-qualified mask (applies to values, refs, or pointers where declared const)
    static constexpr tincup::arity_type const_qualified_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;
  };
}
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
inline constexpr struct variadic_cpo_ftor final :tincup::cpo_base<variadic_cpo_ftor> {
  TINCUP_CPO_TAG("variadic_cpo")
  inline static constexpr bool is_variadic = true;
  // Re-expose base operator() so failures route to diagnostics
  using tincup::cpo_base<variadic_cpo_ftor>::operator();
    // Typed operator() overload - positive case only (generic)
  // Negative cases handled by tagged fallback in cpo_base
  template<typename... T>
    requires tincup::invocable_c<variadic_cpo_ftor, T&...>
  constexpr auto operator()(T&... variadic_args) const
    noexcept(tincup::nothrow_invocable_c<variadic_cpo_ftor, T&...>)
    -> tincup::invocable_t<variadic_cpo_ftor, T&...> {
    return tag_invoke(*this, variadic_args...);
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

// Usage:tincup::is_invocable_v<variadic_cpo_ftor, T&...>


// External generator-provided argument trait specialization for variadic_cpo
// Forward declare primary template to allow specialization even if not included yet
namespace tincup { template<typename Cp, typename...Args> struct cpo_arg_traits; }

namespace tincup {
  template<typename... T>
  struct cpo_arg_traits<variadic_cpo_ftor, T&...> {
    static constexpr bool available = true;
    // Fixed (non-pack) argument count
    static constexpr std::size_t fixed_arity = 0;

    // Helpers to build repeated masks for parameter packs
    static constexpr tincup::arity_type repeat_mask(std::size_t offset, std::size_t count) {
      tincup::arity_type m = tincup::arity_type{0};
      for (std::size_t i = 0; i < count; ++i) m |= (tincup::arity_type{1} << (offset + i));
      return m;
    }

    // Values mask
    static constexpr tincup::arity_type values_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      // Fixed positions
      // Pack positions
        // For packs, category is determined by the declared form of the pack
      return m;
    }() ;

    // Pointers mask
    static constexpr tincup::arity_type pointers_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Lvalue refs mask
    static constexpr tincup::arity_type lvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
        m |= repeat_mask(fixed_arity, sizeof...(T));
      return m;
    }() ;

    // Rvalue refs mask (non-forwarding)
    static constexpr tincup::arity_type rvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Forwarding refs mask
    static constexpr tincup::arity_type forwarding_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Lvalue const refs mask
    static constexpr tincup::arity_type lvalue_const_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Const-qualified mask (applies to values, refs, or pointers where declared const)
    static constexpr tincup::arity_type const_qualified_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;
  };
}
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
inline constexpr struct conditional_process_ftor final :tincup::cpo_base<conditional_process_ftor> {
  TINCUP_CPO_TAG("conditional_process")
  inline static constexpr bool is_variadic = false;
  // Re-expose base operator() so failures route to diagnostics
  using tincup::cpo_base<conditional_process_ftor>::operator();  
  static constexpr struct fast_tag {} fast;
  static constexpr struct safe_tag {} safe;

// Runtime dispatch overload
template<typename T>
constexpr auto operator()(T& data, bool use_fast_path = false) const
  requires (tincup::invocable_c<conditional_process_ftor, T&, fast_tag> && 
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

// Usage:tincup::is_invocable_v<conditional_process_ftor, T&>


// External generator-provided argument trait specialization for conditional_process
// Forward declare primary template to allow specialization even if not included yet
namespace tincup { template<typename Cp, typename...Args> struct cpo_arg_traits; }

namespace tincup {
  template<typename T>
  struct cpo_arg_traits<conditional_process_ftor, T&> {
    static constexpr bool available = true;
    // Fixed (non-pack) argument count
    static constexpr std::size_t fixed_arity = 1;

    // Helpers to build repeated masks for parameter packs
    static constexpr tincup::arity_type repeat_mask(std::size_t offset, std::size_t count) {
      tincup::arity_type m = tincup::arity_type{0};
      for (std::size_t i = 0; i < count; ++i) m |= (tincup::arity_type{1} << (offset + i));
      return m;
    }

    // Values mask
    static constexpr tincup::arity_type values_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      // Fixed positions
      // Pack positions
      return m;
    }() ;

    // Pointers mask
    static constexpr tincup::arity_type pointers_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Lvalue refs mask
    static constexpr tincup::arity_type lvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
 m |= (tincup::arity_type{1} << 0);       return m;
    }() ;

    // Rvalue refs mask (non-forwarding)
    static constexpr tincup::arity_type rvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Forwarding refs mask
    static constexpr tincup::arity_type forwarding_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Lvalue const refs mask
    static constexpr tincup::arity_type lvalue_const_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Const-qualified mask (applies to values, refs, or pointers where declared const)
    static constexpr tincup::arity_type const_qualified_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;
  };
}
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
inline constexpr struct compression_method_ftor final :tincup::cpo_base<compression_method_ftor> {
  TINCUP_CPO_TAG("compression_method")
  inline static constexpr bool is_variadic = false;
  // Re-expose base operator() so failures route to diagnostics
  using tincup::cpo_base<compression_method_ftor>::operator();   
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

// Usage:tincup::is_invocable_v<compression_method_ftor, const T&>


// External generator-provided argument trait specialization for compression_method
// Forward declare primary template to allow specialization even if not included yet
namespace tincup { template<typename Cp, typename...Args> struct cpo_arg_traits; }

namespace tincup {
  template<typename T>
  struct cpo_arg_traits<compression_method_ftor, const T&> {
    static constexpr bool available = true;
    // Fixed (non-pack) argument count
    static constexpr std::size_t fixed_arity = 1;

    // Helpers to build repeated masks for parameter packs
    static constexpr tincup::arity_type repeat_mask(std::size_t offset, std::size_t count) {
      tincup::arity_type m = tincup::arity_type{0};
      for (std::size_t i = 0; i < count; ++i) m |= (tincup::arity_type{1} << (offset + i));
      return m;
    }

    // Values mask
    static constexpr tincup::arity_type values_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      // Fixed positions
      // Pack positions
      return m;
    }() ;

    // Pointers mask
    static constexpr tincup::arity_type pointers_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Lvalue refs mask
    static constexpr tincup::arity_type lvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
 m |= (tincup::arity_type{1} << 0);       return m;
    }() ;

    // Rvalue refs mask (non-forwarding)
    static constexpr tincup::arity_type rvalue_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Forwarding refs mask
    static constexpr tincup::arity_type forwarding_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
      return m;
    }() ;

    // Lvalue const refs mask
    static constexpr tincup::arity_type lvalue_const_refs_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
 m |= (tincup::arity_type{1} << 0);       return m;
    }() ;

    // Const-qualified mask (applies to values, refs, or pointers where declared const)
    static constexpr tincup::arity_type const_qualified_mask = []{
      tincup::arity_type m = tincup::arity_type{0};
 m |= (tincup::arity_type{1} << 0);       return m;
    }() ;
  };
}
```

**Usage:**
```cpp
// Runtime string -> compile-time dispatch (zero overhead after lookup)
auto compressed = compression_method(data, "lz4");

// Direct compile-time usage
auto compressed = compression_method(data, compression_method_ftor::lz4_tag{});
```

</details>

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
