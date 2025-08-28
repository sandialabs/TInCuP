# Makefile - Development Tasks Only

⚠️ **This Makefile is for development tasks, not building the library.**

The `tincup` library is header-only. Use CMake or Meson for project integration.

## Available Tasks

| Command | Description |
|---------|-------------|
| `make help` | Show all available tasks |
| `make test` | Run all tests |
| `make test-patterns` | Run pattern generation tests only |
| `make test-verification` | Run verification tests only |
| `make test-coverage` | Run tests with coverage |
| `make install-dev` | Install development dependencies |
| `make verify-cpos` | Verify CPO patterns in project |
| `make install-git-hook` | Install pre-commit verification hook |
| `make clean` | Clean up generated files |

## For Library Users

- **CMake users**: See `../cmake/README.md`
- **Meson users**: See `../meson/README.md`
- **Other build systems**: Just include `include/tincup/tincup.hpp`