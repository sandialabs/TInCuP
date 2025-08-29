# TInCuP 1.0.3 - Initial Public Release

##  🎉 What is TInCuP?

  TInCuP (Tag Invoke + Customization Points) makes C++ customization points practical by eliminating boilerplate, providing excellent
   error messages, and offering comprehensive tooling support. It bridges the gap between tag_invoke's theoretical benefits and
  real-world usability.

##  ✨ Key Features

###  🚀 Zero-Boilerplate CPO Creation
  - JSON-driven code generation via cpo-generator CLI
  - Automatic concept generation and type safety
  - Static dispatch integration for runtime→compile-time optimization

###  🎯 Enhanced Developer Experience
  - Educational compiler error messages for common mistakes
  - Full IDE integration (VS Code, Vim, CLion)
  - Comprehensive introspection via cpo_traits

###  ⚡ Performance & Compatibility
  - Header-only C++20 library (#include <tincup/tincup.hpp>)
  - Zero runtime overhead with static dispatch utilities
  - Support for GCC 12+, Clang 15+, MSVC 2019 16.11+

##  📦 Installation

  Python Tools:
  pip install TInCuP==1.0.0

  C++ Library: Header-only - just include <tincup/tincup.hpp>

  Build Systems: Native CMake and Meson support

##  🚀 Quick Start

### Generate your first CPO
```
  cpo-generator '{"cpo_name": "process", "args": ["$T&: data"]}' --doxygen
```

### With static dispatch
```
   cpo-generator '{"cpo_name": "execute", "args": ["$T&: input"], "runtime_dispatch": {"type": "bool", "dispatch_arg": "fast_mode", 
  "options": ["fast", "safe"]}}'
```

##  📚 Resources

  - Getting Started: `docs/GETTING_STARTED.md`
  - Build Integration: `docs/BUILD_SYSTEMS.md`
  - Static Dispatch: `docs/STATIC_DISPATCH.md`
  - Examples: `examples/serialize/`

##  🏗️ Project Maturity

  - Comprehensive test suite across multiple compilers
  - OpenSSF Best Practices compliant
  - Extensive documentation and editor integrations
  - Production-ready patterns used at Sandia National Laboratories

##  🙏 Acknowledgments

  TInCuP addresses customization point challenges identified in WG21 papers P1895R0 and P2279R0, providing a practical C++20 solution
  while the language evolves toward better built-in mechanisms.

  ---
  Ready to eliminate CPO boilerplate? Check out the `docs/GETTING_STARTED.md`!

