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
echo "2. Check copyright banners"
echo "3. Run local CI tests"
echo ""

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

echo "Script directory: $SCRIPT_DIR"
echo "Project root: $PROJECT_ROOT"
echo ""

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

# Step 2: Check copyright banners
echo "🏷️  Step 2: Checking copyright banners..."
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

# Step 3: Run local CI
echo "🔧 Step 3: Running local CI tests..."
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