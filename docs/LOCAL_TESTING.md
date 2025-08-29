# Local Testing Guide

This guide explains how to run the complete TInCuP test suite locally, mirroring the GitHub Actions CI pipeline exactly. This helps catch issues before pushing to CI and provides fast feedback during development.

## Quick Start

### Run Complete Local CI Suite
```bash
# Run all tests (mirrors GitHub Actions exactly)
./scripts/scripts/run_local_ci.sh

# Run quick subset (faster, for development)
./scripts/scripts/run_local_ci.sh --quick

# Using Make
make ci-local      # Complete test suite
make ci-quick      # Quick test subset
```

### Run Specific Test Categories
```bash
# Individual build systems
make test-cmake    # Test CMake build only
make test-meson    # Test Meson build only

# Individual test types
make test-python   # Python test suite
make test-examples # Example compilation
make test-headers  # Header-only verification

# Development tests
make test-patterns # Pattern generation tests
make verify-cpos   # CPO pattern verification
```

## What Gets Tested

The local CI suite mirrors **exactly** what runs in GitHub Actions:

### ✅ **Build System Matrix**
- **CMake**: Configuration, build, and consumer smoke tests
- **Meson**: Configuration, build, consumer smoke tests, examples
- **Both GCC and Clang**: Compiler compatibility testing

### ✅ **Python Test Suite**
- Code generator tests (`cpo-generator`)
- Pattern verification tests
- All tool functionality

### ✅ **Smoke Tests**
- CMake consumer projects (external integration)
- Meson consumer projects (subproject integration)
- Header-only library verification

### ✅ **Example Tests**
- Serialize example compilation and execution
- Comparison examples (if present)
- All example CMake/Meson builds

### ✅ **Editor Integration**
- Vim plugin functionality
- VSCode configuration validation
- CLion integration tests

### ✅ **Header-Only Verification**
- Single-header include tests
- Compilation with strict warnings
- Basic functionality verification

## Prerequisites

### Ubuntu/Debian
```bash
sudo apt-get install build-essential cmake meson ninja-build python3 python3-pip
```

### macOS
```bash
brew install cmake meson ninja python3
```

### Arch Linux
```bash
sudo pacman -S base-devel cmake meson ninja python python-pip
```

## Test Output

The local CI suite provides **colored, structured output**:

```
=== TInCuP Local CI Test Suite ===
Repository: /path/to/tincup

=== Checking Prerequisites ===
✓ Found GCC: gcc (Ubuntu 13.3.0) 13.3.0
✓ Found Clang: clang version 15.0.7
✓ Found cmake: cmake version 3.28.3
✓ Found meson: 1.3.2
✓ Found ninja: 1.11.1
✓ Found Python: Python 3.12.3
✓ All prerequisites found

=== Python Tests ===
ℹ Installing Python dependencies...
ℹ Running Python test suite...
✓ Python tests completed

=== Build System Tests ===
ℹ Testing with gcc compiler
ℹ Testing CMake with gcc
ℹ Testing Meson with gcc
✓ CMake with gcc completed
✓ Meson with gcc completed
...
```

## Performance

### Full Test Suite (`./scripts/scripts/run_local_ci.sh`)
- **Time**: ~3-5 minutes
- **Coverage**: 100% CI parity
- **Use case**: Before pushing, release preparation

### Quick Test Suite (`./scripts/scripts/run_local_ci.sh --quick`)  
- **Time**: ~1-2 minutes
- **Coverage**: Core functionality only
- **Use case**: Development iteration, quick verification

### Individual Tests (`make test-*`)
- **Time**: ~10-30 seconds each
- **Coverage**: Specific test category
- **Use case**: Focused debugging, rapid iteration

## Integration with Development Workflow

### Pre-Push Validation
```bash
# Before pushing changes
./scripts/scripts/run_local_ci.sh
git push  # Only if local CI passes
```

### Development Iteration
```bash
# Quick feedback loop during development
make test-cmake     # Test my CMake changes
make test-python    # Test my Python changes
make ci-quick       # Quick overall validation
```

### Debugging CI Failures
```bash
# If CI fails, reproduce locally:
./scripts/scripts/run_local_ci.sh   # Should show the same failure
# Fix the issue
./scripts/scripts/run_local_ci.sh   # Verify fix works
git push            # CI should now pass
```

## Advanced Usage

### Environment Variables
```bash
# Test with specific compiler
CC=clang CXX=clang++ ./scripts/scripts/run_local_ci.sh

# Skip time-consuming tests
SKIP_SLOW_TESTS=1 ./scripts/scripts/run_local_ci.sh
```

### Custom Build Directory
```bash
# Use custom build location
BUILD_DIR=/tmp/tincup_test ./scripts/scripts/run_local_ci.sh
```

### Verbose Output
All test scripts support detailed output for debugging:
```bash
./scripts/scripts/run_local_ci.sh --verbose
make test-cmake V=1
```

## Troubleshooting

### "Command not found" Errors
- **Issue**: Missing build tools
- **Solution**: Install prerequisites (see above)

### "Permission denied" Errors  
- **Issue**: Script not executable
- **Solution**: `chmod +x scripts/run_local_ci.sh`

### Python Import Errors
- **Issue**: TInCuP not installed
- **Solution**: `pip install -e .` or `make install-dev`

### Build Failures
- **Issue**: Compiler/build system issues
- **Solution**: Check that the same command works in CI logs

### Virtual Environment Issues
- **Issue**: Pip install conflicts
- **Solution**: Use `pip install` without `--user` flag in venv

## Benefits

### ✅ **Faster Feedback**
- 2-5 minutes locally vs 10-15 minutes in CI
- No waiting for CI queue
- Immediate iteration

### ✅ **Exact CI Parity**
- Same commands, same flags, same test matrix
- What passes locally will pass in CI
- Eliminates "works locally, fails in CI" issues

### ✅ **Development Efficiency**
- Catch issues before pushing
- Granular testing (test only what changed)
- No broken CI commits

### ✅ **Debugging Support**
- Local reproduction of CI failures
- Detailed logs and output
- Interactive debugging possible

## Best Practices

1. **Run `ci-quick` frequently** during development
2. **Run `ci-local` before every push** to main/develop
3. **Use specific tests** (`test-cmake`, etc.) when debugging
4. **Keep the local environment clean** - use virtual environments
5. **Update regularly** - sync local tools with CI versions

This local testing setup ensures that your changes will pass CI before you push, saving time and preventing broken builds. 🚀