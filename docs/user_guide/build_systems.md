# Build System Integration Guide

This document explains how to integrate `tincup` with different build systems.

## Quick Reference

| Build System | Integration Path | Documentation |
|--------------|------------------|---------------|
| CMake (FetchContent) | Root `CMakeLists.txt` (compatibility) | This document |
| CMake (Manual) | `build_systems/cmake/` | `build_systems/cmake/README.md` |
| Meson | `build_systems/meson/` | `build_systems/meson/README.md` |
| Make (Development) | `build_systems/make/` | `build_systems/make/README.md` |

## CMake Integration

### Option 1: FetchContent (Recommended)

For most users, the root-level `CMakeLists.txt` provides seamless integration:

```cmake
include(FetchContent)

FetchContent_Declare(tincup
  GIT_REPOSITORY https://github.com/sandialabs/TInCuP.git
  GIT_TAG        main)
FetchContent_MakeAvailable(tincup)

target_link_libraries(your_target PRIVATE tincup::tincup)
```

**How it works:** The root `CMakeLists.txt` is a compatibility shim that delegates to the actual configuration in `build_systems/cmake/CMakeLists.txt`. This keeps the root directory clean while maintaining compatibility with existing projects.

### Option 2: Direct Subdir (Advanced)

If you want to explicitly use the organized structure:

```cmake
FetchContent_Declare(tincup
  GIT_REPOSITORY https://github.com/sandialabs/TInCuP.git
  SOURCE_SUBDIR  build_systems/cmake)
FetchContent_MakeAvailable(tincup)
```

### Option 3: Manual Integration

Download the repository and:

```cmake
add_subdirectory(path/to/tincup/build_systems/cmake)
target_link_libraries(your_target PRIVATE tincup::tincup)
```

## Meson Integration

See `build_systems/meson/README.md` for complete Meson documentation.

Quick example:
```meson
tincup_dep = dependency('tincup', fallback : ['tincup', 'tincup_dep'])
executable('my_exe', 'main.cpp', dependencies : [tincup_dep])
```

## Other Build Systems

### Header-Only Usage

`tincup` is header-only, so you can always simply:

1. Copy `include/tincup/tincup.hpp` to your project
2. Add the include path to your build system
3. Ensure C++20 is enabled

### Bazel (Future)

Bazel integration will be added to `build_systems/bazel/` in a future release.

## Troubleshooting

### "CMakeLists.txt not found" Error

If you're getting this error with FetchContent, ensure you're using the root repository URL, not a subdirectory.

**Correct:**
```cmake
GIT_REPOSITORY https://github.com/sandialabs/TInCuP.git
```

**Incorrect:**
```cmake
GIT_REPOSITORY https://github.com/sandialabs/TInCuP.git/build_systems/cmake
```

### "Target not found" Error

Make sure you're linking to `tincup::tincup` (with the namespace):

```cmake
target_link_libraries(your_target PRIVATE tincup::tincup)  # Correct
target_link_libraries(your_target PRIVATE tincup)          # May not work
```

### C++20 Requirement

`tincup` requires C++20. Ensure your project enables it:

```cmake
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

## Project Structure

The organized project structure separates concerns while maintaining compatibility:

```
tincup/
├── CMakeLists.txt              # Compatibility shim for FetchContent
├── build_systems/              # Actual build configurations
│   ├── cmake/CMakeLists.txt    # Real CMake config
│   ├── meson/meson.build       # Real Meson config
│   └── make/Makefile           # Development tasks
└── ...
```

This design eliminates root-level clutter while preserving seamless integration for existing projects.
