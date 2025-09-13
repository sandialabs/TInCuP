#!/bin/bash
# Local CI Test Suite - Mirrors GitHub Actions CI exactly
# Run this to validate changes before pushing to CI

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." &> /dev/null && pwd)"
BUILD_DIR="$PROJECT_ROOT/local_ci_build"
COMPILERS=("gcc" "clang")
# We'll auto-discover available Python versions (3.8–3.12) below
PYTHON_VERSIONS=()

# Helper functions
log_section() {
    echo -e "\n${BLUE}=== $1 ===${NC}\n"
}

log_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

log_error() {
    echo -e "${RED}✗ $1${NC}"
}

log_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

log_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# Check prerequisites
check_prerequisites() {
    log_section "Checking Prerequisites"
    
    local missing_tools=()
    
    # Check compilers
    for compiler in "${COMPILERS[@]}"; do
        if ! command -v "$compiler" &> /dev/null; then
            missing_tools+=("$compiler")
        else
            local version
            if [[ "$compiler" == "gcc" ]]; then
                version=$(gcc --version | head -n1)
                log_success "Found GCC: $version"
            elif [[ "$compiler" == "clang" ]]; then
                version=$(clang --version | head -n1)
                log_success "Found Clang: $version"
            fi
        fi
    done
    
    # Check build tools
    local tools=("cmake" "meson" "ninja")
    for tool in "${tools[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            missing_tools+=("$tool")
        else
            local version
            version=$($tool --version | head -n1)
            log_success "Found $tool: $version"
        fi
    done
    
    # Discover Python versions (prefer 3.8–3.12 if present)
    local candidates=(python3.12 python3.11 python3.10 python3.9 python3.8 python3)
    for py in "${candidates[@]}"; do
        if command -v "$py" &> /dev/null; then
            # Deduplicate
            if [[ ! " ${PYTHON_VERSIONS[*]} " =~ " ${py} " ]]; then
                PYTHON_VERSIONS+=("$py")
                local version
                version=$($py --version 2>&1)
                log_success "Found $py: $version"
            fi
        fi
    done
    if [ ${#PYTHON_VERSIONS[@]} -eq 0 ]; then
        missing_tools+=("python3 (>=3.8)")
    fi
    
    if [ ${#missing_tools[@]} -ne 0 ]; then
        log_error "Missing required tools: ${missing_tools[*]}"
        echo "Please install missing tools and try again."
        echo "Ubuntu/Debian: sudo apt-get install build-essential cmake meson ninja-build python3"
        echo "macOS: brew install cmake meson ninja python3"
        exit 1
    fi
    
    log_success "All prerequisites found"
}

# Clean up previous runs
cleanup_build_dir() {
    log_section "Cleaning Build Directory"
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        log_success "Removed existing build directory"
    fi
    mkdir -p "$BUILD_DIR"
    log_success "Created fresh build directory: $BUILD_DIR"
}

# Generate single header and test it (mirrors single-header-tests job)
run_single_header_tests() {
    log_section "Single Header Generation + Test"

    cd "$PROJECT_ROOT"

    # Use a lightweight venv to ensure quom is available like CI
    local venv_dir="$BUILD_DIR/venv_single_header"
    python3 -m venv "$venv_dir"
    # shellcheck source=/dev/null
    source "$venv_dir/bin/activate"
    python -m pip install --upgrade pip >/dev/null
    pip install -e . >/dev/null
    pip install quom >/dev/null

    # Generate the single header (scripts/generate_single_header.py)
    log_info "Generating single header with quom..."
    (cd scripts && python generate_single_header.py)
    log_success "Single header generated in single_include/tincup.hpp"

    # Compile and run the dedicated single-header test like CI
    log_info "Compiling single header test..."
    g++ -std=c++20 -Wall -Wextra -I. tests/test_single_header.cpp -o "$BUILD_DIR/test_single_header"
    "$BUILD_DIR/test_single_header"
    log_success "Single header test passed"

    deactivate
}

# Python tests (mirrors python-tests job)
run_python_tests() {
    log_section "Python Tests"

    for py in "${PYTHON_VERSIONS[@]}"; do
        log_info "Setting up virtualenv for $($py --version 2>&1)"
        local venv_dir="$BUILD_DIR/venv_${py//./}"
        "$py" -m venv "$venv_dir"
        # shellcheck source=/dev/null
        source "$venv_dir/bin/activate"
        python -m pip install --upgrade pip >/dev/null
        pip install -e "$PROJECT_ROOT" >/dev/null
        if [ -f "$PROJECT_ROOT/tests/requirements.txt" ]; then
            pip install -r "$PROJECT_ROOT/tests/requirements.txt" >/dev/null
        fi

        log_info "Running tests with $(python --version 2>&1)"
        cd "$PROJECT_ROOT"
        python tests/run_tests.py --verbose

        # Also run the pattern verification wrapper similar to CI (non-fatal)
        log_info "Running pattern verification scan (non-fatal)"
        set +e
        python -m cpo_tools.cpo_verification . --strict || true
        set -e

        deactivate
        log_success "Python tests completed for $py"
    done
}

# CMake tests for specific compiler
test_cmake_compiler() {
    local compiler="$1"
    local build_subdir="cmake_${compiler}"
    local cmake_build_dir="$BUILD_DIR/$build_subdir"
    
    log_info "Testing CMake with $compiler"
    
    # Set compiler environment
    if [[ "$compiler" == "gcc" ]]; then
        export CC=gcc
        export CXX=g++
    elif [[ "$compiler" == "clang" ]]; then
        export CC=clang
        export CXX=clang++
    fi
    
    # Configure and build main project
    cd "$PROJECT_ROOT"
    cmake -S . -B "$cmake_build_dir"
    cmake --build "$cmake_build_dir"
    
    # Run CTest tests (including static dispatch test)
    ctest --test-dir "$cmake_build_dir" --output-on-failure
    
    # Smoke test (CMake consumer)
    local smoke_dir="$cmake_build_dir/smoke"
    mkdir -p "$smoke_dir"
    
    cat > "$smoke_dir/CMakeLists.txt" << EOF
cmake_minimum_required(VERSION 3.20)
project(tincup_smoke CXX)
add_subdirectory($PROJECT_ROOT/build_systems/cmake \${CMAKE_BINARY_DIR}/tincup)
add_executable(smoke main.cpp)
target_link_libraries(smoke PRIVATE tincup::tincup)
EOF
    
    cat > "$smoke_dir/main.cpp" << 'EOF'
#include <tincup/tincup.hpp>
int main() { return 0; }
EOF
    
    cmake -S "$smoke_dir" -B "$smoke_dir/build"
    cmake --build "$smoke_dir/build" -v
    
    # Test the smoke executable
    "$smoke_dir/build/smoke"
    
    log_success "CMake with $compiler completed"
}

# Meson tests for specific compiler  
test_meson_compiler() {
    local compiler="$1"
    local build_subdir="meson_${compiler}"
    local meson_build_dir="$BUILD_DIR/$build_subdir"
    
    log_info "Testing Meson with $compiler"
    
    # Set compiler environment
    if [[ "$compiler" == "gcc" ]]; then
        export CC=gcc
        export CXX=g++
    elif [[ "$compiler" == "clang" ]]; then
        export CC=clang
        export CXX=clang++
    fi
    
    # Configure and build main project
    cd "$PROJECT_ROOT"
    meson setup "$meson_build_dir" build_systems/meson -Denable_examples=true
    meson compile -C "$meson_build_dir"
    
    # Run Meson tests (including static dispatch test)
    meson test -C "$meson_build_dir" --print-errorlogs
    
    # Smoke test (Meson consumer)
    local smoke_dir="$BUILD_DIR/meson_smoke_${compiler}"
    mkdir -p "$smoke_dir/subprojects"
    
    cat > "$smoke_dir/meson.build" << 'EOF'
project('tincup_smoke', 'cpp', default_options: ['cpp_std=c++20'])
tincup_proj = subproject('tincup', default_options: ['cpp_std=c++20'])
tincup_dep = tincup_proj.get_variable('tincup_dep')
executable('smoke', 'main.cpp', dependencies : [tincup_dep])
EOF
    
    cat > "$smoke_dir/main.cpp" << 'EOF'
#include <tincup/tincup.hpp>
int main() { return 0; }
EOF
    
    # Link the Meson subproject
    ln -s "$PROJECT_ROOT/build_systems/meson" "$smoke_dir/subprojects/tincup"
    
    meson setup "$smoke_dir/build" "$smoke_dir"
    meson compile -C "$smoke_dir/build" -v
    
    # Test the smoke executable
    "$smoke_dir/build/smoke"
    
    log_success "Meson with $compiler completed"
}

# Build system tests (mirrors cmake-meson job)
run_build_system_tests() {
    log_section "Build System Tests"
    
    for compiler in "${COMPILERS[@]}"; do
        log_info "Testing with $compiler compiler"
        
        # Test CMake
        test_cmake_compiler "$compiler"
        
        # Test Meson
        test_meson_compiler "$compiler"
    done
    
    log_success "All build system tests completed"
}

# Editor integration tests (mirrors editor-integration job)
run_editor_integration_tests() {
    log_section "Editor Integration Tests"
    
    # Install Python tooling (needed for editor tests)
    cd "$PROJECT_ROOT"
    python3 -m pip install --upgrade pip
    pip install -e .
    
    # Run editor integration tests
    if [ -f "$PROJECT_ROOT/tests/vim_integration_test.sh" ]; then
        log_info "Running Vim integration tests..."
        bash "$PROJECT_ROOT/tests/vim_integration_test.sh"
        log_success "Vim integration tests passed"
    fi
    
    if [ -f "$PROJECT_ROOT/tests/vscode_config_check.sh" ]; then
        log_info "Running VSCode configuration tests..."
        bash "$PROJECT_ROOT/tests/vscode_config_check.sh"
        log_success "VSCode configuration tests passed"
    fi
    
    if [ -f "$PROJECT_ROOT/tests/clion_integration_test.sh" ]; then
        log_info "Running CLion integration tests..."
        bash "$PROJECT_ROOT/tests/clion_integration_test.sh"
        log_success "CLion integration tests passed"
    fi
    
    log_success "Editor integration tests completed"
}

# Test examples (if they exist)
run_example_tests() {
    log_section "Example Tests"
    
    cd "$PROJECT_ROOT"
    
    # Test serialize example
    if [ -f "examples/serialize/test_serialization.cpp" ]; then
        log_info "Testing serialize example..."
        g++ -std=c++20 -I include examples/serialize/test_serialization.cpp -o "$BUILD_DIR/serialize_test"
        "$BUILD_DIR/serialize_test"
        log_success "Serialize example test passed"
    fi
    
    # Test comparison examples if they exist
    if [ -d "examples/comparison" ]; then
        log_info "Testing comparison examples..."
        
        # Test OOP approach
        if [ -f "examples/comparison/oop_approach/inheritance_serialization.cpp" ]; then
            g++ -std=c++20 -I include examples/comparison/oop_approach/inheritance_serialization.cpp -o "$BUILD_DIR/oop_test"
            "$BUILD_DIR/oop_test"
            log_success "OOP comparison test passed"
        fi
        
        # Test CPO approach  
        if [ -f "examples/comparison/cpo_approach/cpo_serialization.cpp" ]; then
            g++ -std=c++20 -I include examples/comparison/cpo_approach/cpo_serialization.cpp -o "$BUILD_DIR/cpo_test"
            "$BUILD_DIR/cpo_test"
            log_success "CPO comparison test passed"
        fi
    fi
    
    log_success "Example tests completed"
}

# Header-only verification
run_header_verification() {
    log_section "Header-Only Library Verification"
    
    log_info "Testing single-header include..."
    cat > "$BUILD_DIR/header_test.cpp" << 'EOF'
#include <tincup/tincup.hpp>

// Test that we can create a simple CPO (must be at namespace scope)
struct test_ftor : tincup::cpo_base<test_ftor> {
    TINCUP_CPO_TAG("test")
};

// Test that the header compiles and basic functionality works
int main() {
    // Test basic concepts and type traits
    static_assert(tincup::always_false_v<int> == false);
    
    // Test that CPO instantiation works
    [[maybe_unused]] test_ftor test_instance;
    
    return 0;
}
EOF
    
    # Test with both compilers
    for compiler in "${COMPILERS[@]}"; do
        log_info "Testing header-only with $compiler..."
        if [[ "$compiler" == "gcc" ]]; then
            g++ -std=c++20 -I include -Wall -Wextra -Werror -Wno-unused-function "$BUILD_DIR/header_test.cpp" -o "$BUILD_DIR/header_test_gcc"
            "$BUILD_DIR/header_test_gcc"
        elif [[ "$compiler" == "clang" ]]; then
            clang++ -std=c++20 -I include -Wall -Wextra -Werror -Wno-unused-function "$BUILD_DIR/header_test.cpp" -o "$BUILD_DIR/header_test_clang" 
            "$BUILD_DIR/header_test_clang"
        fi
        log_success "Header test with $compiler passed"
    done
    
    log_success "Header-only verification completed"
}

# Generator compile smoke test (mirrors cpo-verification workflow)
run_cpo_verification_compile() {
    log_section "Generator Compile Smoke Test"

    cd "$PROJECT_ROOT"
    mkdir -p "$BUILD_DIR/test_generated"

    # Generate a simple CPO header via the generator
    python3 -m cpo_tools.cpo_generator '{"cpo_name": "test_compile", "args": ["$T&: arg"]}' > "$BUILD_DIR/test_generated/test.hpp"

    # Minimal test TU
    cat > "$BUILD_DIR/test_generated/test.cpp" << 'EOF'
#include "tincup/tincup.hpp"
#include "test.hpp"
int main() { return 0; }
EOF

    # Choose a GCC if available, fall back to clang++
    CXXBIN=""
    if command -v g++-11 >/dev/null 2>&1; then CXXBIN=g++-11; 
    elif command -v g++ >/dev/null 2>&1; then CXXBIN=g++; 
    elif command -v clang++ >/dev/null 2>&1; then CXXBIN=clang++; 
    else
        log_error "No suitable C++ compiler found for generator smoke test"
        exit 1
    fi

    # Compile with YOUR_NAMESPACE cleared so the generated trait specialization binds
    (cd "$BUILD_DIR/test_generated" && "$CXXBIN" -std=c++20 -DYOUR_NAMESPACE= -I"$PROJECT_ROOT/include" -c test.cpp)
    log_success "Generator compile smoke test passed with $CXXBIN"
}

# Summary function
print_summary() {
    log_section "Test Summary"
    echo "All local CI tests completed successfully!"
    echo ""
    echo "Tests run:"
    echo "  ✓ Prerequisites check"
    echo "  ✓ Python test suite"
    echo "  ✓ CMake build tests (GCC + Clang)"
    echo "  ✓ Meson build tests (GCC + Clang)" 
    echo "  ✓ Smoke tests (CMake + Meson consumers)"
    echo "  ✓ Editor integration tests"
    echo "  ✓ Example tests"
    echo "  ✓ Header-only verification"
    echo ""
    echo "Your changes are ready for CI! 🚀"
}

# Main execution
main() {
    log_section "TInCuP Local CI Test Suite"
    echo "This script mirrors the GitHub Actions CI pipeline locally"
    echo "Repository: $PROJECT_ROOT"
    echo ""
    
    check_prerequisites
    cleanup_build_dir
    run_single_header_tests
    run_python_tests
    run_build_system_tests
    run_editor_integration_tests
    run_example_tests
    run_header_verification
    run_cpo_verification_compile
    print_summary
}

# Handle script arguments
case "${1:-}" in
    "--help"|"-h")
        echo "Usage: $0 [--help|--quick]"
        echo ""
        echo "Options:"
        echo "  --help, -h     Show this help message"
        echo "  --quick        Run quick tests only (skip some time-consuming tests)"
        echo ""
        echo "This script runs the complete TInCuP test suite locally, mirroring"
        echo "the GitHub Actions CI pipeline. Run this before pushing to catch"
        echo "issues early."
        exit 0
        ;;
    "--quick")
        log_info "Running quick test suite..."
        # For quick mode, skip some time-consuming tests
        COMPILERS=("gcc")  # Test only one compiler
        ;;
esac

# Execute main function
main
