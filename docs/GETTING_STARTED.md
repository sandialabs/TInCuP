# Getting Started with tincup

Welcome to `tincup` - a header-only C++20 library for creating Customization Point Objects with automated tooling.

## Choose Your Path

### 🚀 **I Just Want to Use the Library**

`tincup` is header-only. Simply:

```cpp
#include <tincup/tincup.hpp>
```

**Build System Integration:**
- **CMake**: See `build_systems/cmake/README.md`
- **Meson**: See `build_systems/meson/README.md` 
- **Other**: Just include the header

### ⚒️ **I Want to Generate CPO Boilerplate**

Install the code generation tools:

```bash
pip install -e .
```

Then use the generator:

```bash
# Generate a CPO
cpo-generator '{"cpo_name": "my_cpo", "args": ["$T&: target"]}'

# See all semantic patterns  
cpo-generator --llm-help
```

### 🔧 **I Want IDE Integration**

Choose your editor:

- **VSCode**: See `editor_integration/vscode/README.md`
- **Vim**: See `editor_integration/vim/README.md`
- **CLion**: See `editor_integration/clion/README.md`

### 📖 **Doxygen Integration**

The generated CPO headers use custom tags (`@cpo`, `@cpo_example`, `@tag_invoke_impl`).
To enable these in your documentation, see [`docs/DOXYGEN.md`](DOXYGEN.md).

### 🧪 **I Want to Contribute/Develop**

```bash
# Development setup
make -f build_systems/make/Makefile install-dev

# Run tests
make -f build_systems/make/Makefile test

# Verify CPO patterns
make -f build_systems/make/Makefile verify-cpos
```

## Quick Examples

### Using the Library

```cpp
#include <tincup/tincup.hpp>

// Define your CPO
inline constexpr struct my_cpo_ftor final : cpo_base<my_cpo_ftor> {
  TINCUP_CPO_TAG("my_cpo")
  
  template<typename T>
    requires tag_invocable_c<my_cpo_ftor, T&>
    constexpr auto operator()(T& obj) const
    noexcept(nothrow_tag_invocable_c<my_cpo_ftor, T&>) -> tag_invocable_t<my_cpo_ftor, T&> {
      return tag_invoke(*this, obj);
    }
    
  template<typename T>
    requires (!tag_invocable_c<my_cpo_ftor, T&>)
    constexpr void operator()(T& obj) const {
      this->fail(obj);
    }
} my_cpo;

// Implement for your type
struct MyType { int value; };

constexpr auto tag_invoke(my_cpo_ftor, MyType& obj) {
    return obj.value * 2;
}

// Use it
MyType obj{42};
auto result = my_cpo(obj);  // Returns 84
```

### Generating CPOs

```bash
# Generic CPO
cpo-generator '{"cpo_name": "process", "args": ["$T&: target", "$const U&: source"]}'

# Semantic/LLM mode
cpo-generator '{"cpo_name": "add_in_place", "operation_type": "mutating_binary"}' --doxygen
```

## Project Structure

```
tincup/
├── include/tincup/         # Header-only library
├── cpo_tools/              # Code generation tools
├── build_systems/          # CMake, Meson, Make
├── editor_integration/     # VSCode, Vim, CLion integrations
├── tests/                  # Comprehensive test suite
└── docs/                   # Documentation and guides
```

## Next Steps

1. **Library Users**: Jump to your build system's README in `build_systems/`
2. **Tool Users**: Try `cpo-generator --llm-help` to see generation patterns
3. **IDE Users**: Set up your editor integration
4. **Contributors**: Run the test suite and explore the verification tools

For detailed documentation, explore the `docs/` directory and the READMEs in each integration folder.
