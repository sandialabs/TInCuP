#!/bin/bash
# Test script to validate MSVC CI configuration locally
# This script checks that the MSVC CI job definition is valid and complete

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CI_FILE="$PROJECT_ROOT/.github/workflows/ci.yml"

echo "Testing MSVC CI configuration..."

# Test 1: Check CI file exists and contains MSVC job
echo "✓ Checking CI file contains MSVC job..."
if ! grep -q "msvc-cpp20:" "$CI_FILE"; then
    echo "✗ MSVC CI job not found in workflow"
    exit 1
fi

# Test 2: Check required MSVC job steps
echo "✓ Checking MSVC job has required steps..."
required_steps=(
    "Setup MSVC"
    "Install CMake and Ninja"
    "Install Python dependencies"
    "Test CPO generation"
    "Create MSVC test project"
    "Create simplified MSVC test source"
    "Configure MSVC build"
    "Build with MSVC"
    "Run MSVC tests"
    "Test CPO pattern verification"
)

for step in "${required_steps[@]}"; do
    if ! grep -q "name: $step" "$CI_FILE"; then
        echo "✗ Required step '$step' not found in MSVC job"
        exit 1
    fi
done

# Test 3: Check MSVC job uses correct actions
echo "✓ Checking MSVC job uses correct actions..."
if ! grep -q "uses: ilammy/msvc-dev-cmd@v1" "$CI_FILE"; then
    echo "✗ MSVC setup action not found"
    exit 1
fi

if ! grep -q "uses: lukka/get-cmake@v3.29.4" "$CI_FILE"; then
    echo "✗ CMake setup action not found"
    exit 1
fi

# Test 4: Check MSVC job has strategy matrix
echo "✓ Checking MSVC job has build type matrix..."
if ! grep -A5 "msvc-cpp20:" "$CI_FILE" | grep -q "strategy:"; then
    echo "✗ MSVC job missing strategy matrix"
    exit 1
fi

if ! grep -q "build_type: \[Debug, Release\]" "$CI_FILE"; then
    echo "✗ MSVC job missing Debug/Release matrix"
    exit 1
fi

# Test 5: Check MSVC smoke test file exists
echo "✓ Checking MSVC smoke test file exists..."
if [[ ! -f "$SCRIPT_DIR/msvc_smoke_test.cpp" ]]; then
    echo "✗ MSVC smoke test file not found"
    exit 1
fi

# Test 6: Check MSVC smoke test contains required tests
echo "✓ Checking MSVC smoke test content..."
msvc_test_file="$SCRIPT_DIR/msvc_smoke_test.cpp"
required_test_features=(
    "msvc_basic_tests"
    "always_false_v"
    "#include <concepts>"
    "#include <type_traits>"
    "static_assert"
    "std::is_trivially_constructible_v"
)

for feature in "${required_test_features[@]}"; do
    if ! grep -q "$feature" "$msvc_test_file"; then
        echo "✗ MSVC test missing feature: $feature"
        exit 1
    fi
done

# Test 7: Check MSVC CMake test file
echo "✓ Checking MSVC CMake test configuration..."
if [[ ! -f "$SCRIPT_DIR/msvc_test_CMakeLists.txt" ]]; then
    echo "✗ MSVC CMake test file not found"
    exit 1
fi

cmake_test_file="$SCRIPT_DIR/msvc_test_CMakeLists.txt"
if ! grep -q "/std:c++20 /permissive-" "$cmake_test_file"; then
    echo "✗ MSVC CMake test missing C++20 flags"
    exit 1
fi

# Test 8: Check that MSVC job uses PowerShell
echo "✓ Checking MSVC job uses PowerShell..."
if ! grep -A20 "msvc-cpp20:" "$CI_FILE" | grep -q "shell: pwsh"; then
    echo "✗ MSVC job not using PowerShell"
    exit 1
fi

# Test 9: Check MSVC job tests both Debug and Release
echo "✓ Checking MSVC job tests both build configurations..."
if ! grep -q "CMAKE_BUILD_TYPE=\${{ matrix.build_type }}" "$CI_FILE"; then
    echo "✗ MSVC job not using build type matrix variable"
    exit 1
fi

# Test 10: Check MSVC job includes CPO generation test
echo "✓ Checking MSVC job tests CPO generation..."
if ! grep -q "test_msvc" "$CI_FILE" || ! grep -q "cpo-generator" "$CI_FILE"; then
    echo "✗ MSVC job missing CPO generation test"
    exit 1
fi

# Test 11: Check PowerShell JSON syntax is correct
echo "✓ Checking PowerShell uses proper JSON variable syntax..."
if grep -q '{\\".*\\"}' "$CI_FILE"; then
    echo "✗ MSVC job still uses problematic JSON escaping"
    exit 1
fi

if ! grep -q '\$jsonInput.*cpo_name' "$CI_FILE"; then
    echo "✗ MSVC job missing proper PowerShell JSON variable syntax"
    exit 1
fi

echo ""
echo "🎉 All MSVC CI configuration tests passed!"
echo ""
echo "Verified components:"
echo "  ✓ MSVC CI job definition in workflow"
echo "  ✓ All required build steps present"
echo "  ✓ Correct GitHub Actions usage"
echo "  ✓ Debug/Release build matrix"
echo "  ✓ MSVC simplified smoke test"
echo "  ✓ CMake configuration for MSVC"
echo "  ✓ PowerShell scripting for Windows"
echo "  ✓ C++20 basic feature testing"
echo "  ✓ CPO generation and verification"
echo ""
echo "MSVC CI is ready to test:"
echo "  • MSVC 2019 16.11+ / 2022+ compatibility"
echo "  • Basic C++20 feature support"
echo "  • Debug and Release build modes"
echo "  • TInCuP library compilation"
echo "  • CPO generation and verification"
echo "  • Simplified runtime testing"