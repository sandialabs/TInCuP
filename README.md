# TInCuP - Tag Invoke + Customization Points

<div align="center">
  <img src="tincup.png" alt="TInCuP Logo" width="200"/>
</div>

[![CI](https://github.com/sandialabs/TInCuP/actions/workflows/ci.yml/badge.svg)](https://github.com/sandialabs/TInCuP/actions/workflows/ci.yml)
[![OpenSSF Best Practices](https://www.bestpractices.dev/projects/11107/badge)](https://www.bestpractices.dev/projects/11107)
[![OpenSSF Scorecard](https://api.securityscorecards.dev/projects/github.com/sandialabs/TInCuP/badge)](https://securityscorecards.dev/viewer/?uri=github.com/sandialabs/TInCuP)
[![PyPI](https://img.shields.io/pypi/v/TInCuP.svg)](https://pypi.org/project/TInCuP/)
[![Python Versions](https://img.shields.io/pypi/pyversions/TInCuP.svg)](https://pypi.org/project/TInCuP/)
[![CMake](https://img.shields.io/badge/CMake-supported-blue?logo=cmake)](docs/user_guide/build_systems.md)
[![Meson](https://img.shields.io/badge/Meson-supported-brightgreen)](docs/user_guide/build_systems.md)
[![VS%20Code](https://img.shields.io/badge/VS%20Code-integration-007ACC?logo=visualstudiocode)](docs/user_guide/ide_integration.md)
[![Vim](https://img.shields.io/badge/Vim-plugin-019733?logo=vim)](docs/user_guide/ide_integration.md)
[![CLion](https://img.shields.io/badge/CLion-integration-0074D9?logo=jetbrains)](docs/user_guide/ide_integration.md)
[![GCC](https://img.shields.io/badge/GCC-C%2B%2B20-success?logo=gnu)](#supported-compilers)
[![Clang](https://img.shields.io/badge/Clang-C%2B%2B20-success?logo=llvm)](#supported-compilers)
[![MSVC](https://img.shields.io/badge/MSVC-C%2B%2B20-blue?logo=visualstudio)](#supported-compilers)

TInCuP helps you add clean, extensible “hooks” to your C++ library. Describe what you want in a simple spec, generate correct code, and get readable compiler messages when something goes wrong.

## Why Should I Care About Customization Points (and TInCuP?)

Have you ever been in a situation like the following?

- You are developing `ImpressiveLib`, a very impressive library.
- You would like `ImpressiveLib` to be generic and work with a variety of third party types.
- For example, it should optionally use `AwesomeType`, which is defined in `CoolLib`, an open source library that you *don't* own. 
- You don't want to write `ImpressiveLib`'s source code to use `#include <coollib>`. 
- You are tempted to modify CoolLib in some way such as injecting ImpressiveLib types into it **(DON'T!)**.
- This is where customization points can help!

See TInCuP in action in [Real Vector Framework](http://github.com/sandialabs/RealVectorFramework.git)

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

## Features

*   **Header-Only C++20 Library**: A single header `<tincup/tincup.hpp>` provides all necessary components.
*   **Static Dispatch Integration**: Built-in compile-time dispatch utilities (BoolDispatch, StringDispatch) that convert runtime configuration into zero-overhead template specializations.
*   **Enhanced Error Diagnostics**: Comprehensive misuse detection system catching 5 major categories of CPO mistakes with helpful, educational error messages (even more sophisticated with C++23). Performance opt-outs available.
*   **CMake and Meson Support**: First-class integration with both build systems for easy inclusion in any project.
*   **Powerful Code Generator**: A Python-based command-line tool, `cpo-generator`, to automatically generate CPO boilerplate from simple JSON definitions, including static dispatch-enabled CPOs.
*   **IDE/Editor Integration**: Full support for Vim, VSCode, and CLion with plugins, templates, and external tools.
*   **Modern & Friendly**: Designed with modern C++ and Python practices to be user-friendly and easy to extend.

## Try Before Installing

Want to experiment with TInCuP before installing? Use our Compiler Explorer examples:

- Compiler flags used: `-std=c++20 -O2 -DTINCUP_DIAGNOSTIC_LEVEL=3`
* [Basic Printing](https://godbolt.org/z/3945vq437)
* [Bool Dispatch](https://godbolt.org/z/K1xcqqPad)
* Enhanced Error Diagnostics:
  - [Forgetting to dereference](https://godbolt.org/z/TKbvz8fMd)

These examples demonstrate that TInCuP:
- ✅ Works with all major compilers (GCC, Clang, MSVC)
- ✅ Produces optimal assembly (check the "Add New → Opt Output" panel)
- ✅ Provides clear error messages

## Installation

- Python tools (latest): `pip install TInCuP` (installs the `cpo-generator` CLI)
- Python tools (this release): `pip install TInCuP==1.0.4`
- From source (development): `pip install -e .`

Troubleshooting CLI on macOS/Linux:
- If `cpo-generator` is not found after install, ensure your user scripts directory is on `PATH`:
  - `export PATH="$(python3 -m site --user-base)/bin:$PATH"`
  - Add the line above to your shell profile (e.g., `~/.zshrc` or `~/.bashrc`).
- Alternatively, use `pipx` to manage CLI tools: `pipx install TInCuP` or `pipx install TInCuP==1.0.4`
- The module form always works: `python3 -m cpo_tools.cpo_generator --help`.

## Quick Start & Documentation

**New to the project?** See our **[Getting Started Guide](docs/user_guide/getting_started.md)**.

For full documentation, including guides on the C++ library, code generator, build system integration, and more, please see the **[TInCuP Documentation](docs/index.md)**.

**Want to see examples?** Check out:
- **[Generated CPO Examples](docs/examples.md)** - Auto-generated examples showing different CPO patterns
- **[Working Examples](examples/)** - Complete, buildable projects demonstrating real-world usage

## Get, Feedback, Contribute

- **Obtain**:
  - PyPI (latest): `pip install TInCuP`
  - PyPI (this release): `pip install TInCuP==1.0.4`
  - Source: `git clone https://github.com/sandialabs/TInCuP.git && cd TInCuP && pip install -e .`
- **Feedback** (bugs/enhancements): Open an issue: https://github.com/sandialabs/TInCuP/issues
- **Security** reports: see [SECURITY.md](SECURITY.md)
- **Contribute**: Read [CONTRIBUTING.md](docs/development/contributing.md) and follow our [Code of Conduct](CODE_OF_CONDUCT.md)

## Supported Compilers

TInCuP is tested on CI with strict C++20 compliance:

- **GCC**: 12+ (tested on Ubuntu, comprehensive C++20 support)
- **Clang**: 15+ (tested on Ubuntu/macOS, excellent C++20 support)  
- **MSVC**: 2019 16.11+ / 2022+ (tested on Windows, `/std:c++20 /permissive-`)

## References

- [**N4381** Customization Point Design in C++11 and Beyond](https://ericniebler.com/2014/10/21/customization-point-design-in-c11-and-beyond/)
- [Suggested Design for Customization Points](https://open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4381.html)
- [**P1895R0** `tag_invoke`: A general pattern for supporting customisable functions](https://open-std.org/JTC1/SC22/WG21/docs/papers/2019/p1895r0.pdf)
- [**P2279R0** We need a language mechanism for customization points](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2021/p2279r0.html) 

## License

This project is licensed under the BSD 3-Clause License. See the `LICENSE` file for details.
