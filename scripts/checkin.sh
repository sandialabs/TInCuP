#!/bin/bash

# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia, 
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

set -e  # Exit on any error

echo "=== TInCuP Pre-Checkin Script ==="
echo "This script will:"
echo "1. Generate the single header file"
echo "2. Update README install version"
echo "3. Generate examples documentation"
echo "4. Check copyright banners"
echo "5. Run local CI tests"
echo ""

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "Script directory: $SCRIPT_DIR"
echo "Project root: $PROJECT_ROOT"
echo ""

# Pre-step: clean stale local build directories to avoid false positives
echo "🧹 Pre-step: Cleaning local build directories..."
{
    # Common local build trees created by manual builds or previous runs
    CANDIDATE_DIRS=(
        "$PROJECT_ROOT/build"
        "$PROJECT_ROOT/build_tmp"
        "$PROJECT_ROOT/local_ci_build"
        "$PROJECT_ROOT/build_systems/cmake/build"
    )
    for d in "${CANDIDATE_DIRS[@]}"; do
        if [ -d "$d" ]; then
            echo "  - Removing $d"
            rm -rf "$d"
        fi
    done
}
echo "✅ Clean step complete"
echo ""

# Pre-step: Compatibility CMake subdir rebuild (captures build/build_systems/cmake/build.log)
if command -v cmake >/dev/null 2>&1; then
    echo "🏗️  Pre-step: Running compatibility CMake subdir rebuild..."
    COMPAT_BDIR="$PROJECT_ROOT/build/build_systems/cmake"
    rm -rf "$COMPAT_BDIR"
    mkdir -p "$COMPAT_BDIR"
    {
        echo "=== CMake Configure (build_systems/cmake) ==="
        cmake -S "$PROJECT_ROOT/build_systems/cmake" -B "$COMPAT_BDIR"
        echo "=== CMake Build (build_systems/cmake) ==="
        cmake --build "$COMPAT_BDIR" -j
    } | tee "$COMPAT_BDIR/build.log"
    echo "✅ Compatibility rebuild complete → $COMPAT_BDIR/build.log"
    echo ""
else
    echo "⚠ Skipping compatibility CMake subdir rebuild (cmake not found)"
    echo ""
fi

# Step 1: Generate single header
echo "📦 Step 1: Generating single header..."
cd "$SCRIPT_DIR"
python generate_single_header.py
if [ $? -eq 0 ]; then
    echo "✅ Single header generated successfully"
else
    echo "❌ Single header generation failed"
    exit 1
fi
echo ""

# Step 2: Update README install version
echo "🔢 Step 2: Updating README install version from VERSION..."
cd "$PROJECT_ROOT"
python3 scripts/update_readme_version.py
if [ $? -eq 0 ]; then
    echo "✅ README install version updated"
else
    echo "❌ Updating README install version failed"
    exit 1
fi
echo ""

# Step 3: Generate examples documentation
echo "📝 Step 3: Generating examples documentation..."
cd "$PROJECT_ROOT"
python3 scripts/generate_examples_doc.py
if [ $? -eq 0 ]; then
    echo "✅ Examples documentation generated"
else
    echo "❌ Generating examples documentation failed"
    exit 1
fi
echo ""

# Step 4: Check copyright banners
echo "🏷️  Step 4: Checking copyright banners..."
cd "$PROJECT_ROOT"
python scripts/banner_check.py
if [ $? -eq 0 ]; then
    echo "✅ Copyright banners are compliant"
else
    echo "❌ Copyright banner check failed"
    echo "Run 'python scripts/banner_check.py --fix' to automatically fix banner issues"
    exit 1
fi
echo ""

# Step 5: Run local CI
echo "🔧 Step 5: Running local CI tests..."
cd "$SCRIPT_DIR"
./run_local_ci.sh
if [ $? -eq 0 ]; then
    echo "✅ Local CI tests passed"
else
    echo "❌ Local CI tests failed"
    exit 1
fi
echo ""

echo "🎉 All pre-checkin checks passed! Ready to commit."
echo ""
echo "Next steps:"
echo "  - Review your changes with 'git diff'"
echo "  - Stage your changes with 'git add'"
echo "  - Commit with 'git commit'"
