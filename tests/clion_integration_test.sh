#!/bin/bash
# CLion Integration Test Script
# Tests that CLion integration files are valid and properly structured

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
CLION_DIR="$PROJECT_ROOT/editor_integration/clion"

echo "Testing CLion integration files..."

# Test 1: Check all required files exist
echo "✓ Checking required files exist..."
required_files=(
    "external_tools.xml"
    "live_templates.xml"
    "file_templates/CPO_Header.hpp"
    "file_templates/CPO_Implementation.cpp"
    "install.sh"
    "README.md"
)

for file in "${required_files[@]}"; do
    if [[ ! -f "$CLION_DIR/$file" ]]; then
        echo "✗ Missing required file: $file"
        exit 1
    fi
done

# Test 2: Validate XML files are well-formed
echo "✓ Checking XML files are well-formed..."
if command -v xmllint &> /dev/null; then
    xmllint --noout "$CLION_DIR/external_tools.xml" 2>/dev/null || {
        echo "✗ external_tools.xml is not well-formed XML"
        exit 1
    }
    
    xmllint --noout "$CLION_DIR/live_templates.xml" 2>/dev/null || {
        echo "✗ live_templates.xml is not well-formed XML"
        exit 1
    }
else
    echo "⚠ xmllint not available, skipping XML validation"
fi

# Test 3: Check external tools contain expected tools
echo "✓ Checking external tools configuration..."
expected_tools=(
    "Generate CPO"
    "Generate CPO with Doxygen" 
    "Generate CPO Interactive"
    "Verify CPO Patterns"
    "Run CPO Tests"
    "Show LLM Help"
)

for tool in "${expected_tools[@]}"; do
    if ! grep -q "name=\"$tool\"" "$CLION_DIR/external_tools.xml"; then
        echo "✗ External tool '$tool' not found in configuration"
        exit 1
    fi
done

# Test 4: Check live templates contain expected templates  
echo "✓ Checking live templates configuration..."
expected_templates=(
    "cpo-generic-unary"
    "cpo-generic-binary"
    "tag-invoke-impl"
    "cpo-include"
)

for template in "${expected_templates[@]}"; do
    if ! grep -q "name=\"$template\"" "$CLION_DIR/live_templates.xml"; then
        echo "✗ Live template '$template' not found in configuration"
        exit 1
    fi
done

# Test 5: Check file templates contain template variables
echo "✓ Checking file templates..."
header_template="$CLION_DIR/file_templates/CPO_Header.hpp"
impl_template="$CLION_DIR/file_templates/CPO_Implementation.cpp"

# Check header template has required variables
required_header_vars=(
    "CPO_NAME"
    "TEMPLATE_PARAMS"
    "FUNCTION_PARAMS"
    "NAMESPACE_OPEN"
    "NAMESPACE_CLOSE"
)

for var in "${required_header_vars[@]}"; do
    if ! grep -q "\${$var}" "$header_template"; then
        echo "✗ Header template missing variable: \${$var}"
        exit 1
    fi
done

# Check implementation template has required variables
required_impl_vars=(
    "HEADER_NAME"
    "CPO_NAME"
    "EXAMPLE_PARAMS"
    "RETURN_TYPE"
)

for var in "${required_impl_vars[@]}"; do
    if ! grep -q "\${$var}" "$impl_template"; then
        echo "✗ Implementation template missing variable: \${$var}"
        exit 1
    fi
done

# Test 6: Check install script is executable
echo "✓ Checking install script..."
if [[ ! -x "$CLION_DIR/install.sh" ]]; then
    echo "✗ install.sh is not executable"
    exit 1
fi

# Test 7: Test install script dry-run
echo "✓ Testing install script dry-run..."
cd "$CLION_DIR"
temp_dir=$(mktemp -d)
mkdir -p "$temp_dir"
if ! ./install.sh --dry-run --config-dir "$temp_dir"; then
    echo "✗ Install script dry-run failed"
    rm -rf "$temp_dir"
    exit 1
fi
rm -rf "$temp_dir"

# Test 8: Check external tools reference correct commands
echo "✓ Checking external tools reference correct commands..."
if ! grep -q "cpo-generator" "$CLION_DIR/external_tools.xml"; then
    echo "✗ External tools should reference 'cpo-generator' command"
    exit 1
fi

if ! grep -q "cpo_tools.cpo_verification" "$CLION_DIR/external_tools.xml"; then
    echo "✗ External tools should reference verification module"
    exit 1
fi

# Test 9: Check README has all sections
echo "✓ Checking README completeness..."
readme="$CLION_DIR/README.md"
required_sections=(
    "Quick Setup"
    "What's Included"
    "Usage Examples"
    "Keyboard Shortcuts"
    "Integration with CMake"
    "Troubleshooting"
)

for section in "${required_sections[@]}"; do
    if ! grep -q "## $section" "$readme"; then
        echo "✗ README missing section: $section"
        exit 1
    fi
done

# Test 10: Check that live templates use proper C++ context
echo "✓ Checking live templates context..."
if ! grep -q "CPP.*true" "$CLION_DIR/live_templates.xml"; then
    echo "✗ Live templates should specify C++ context"
    exit 1
fi

echo ""
echo "🎉 All CLion integration tests passed!"
echo ""
echo "Files tested:"
for file in "${required_files[@]}"; do
    echo "  ✓ $file"
done

echo ""
echo "Integration features verified:"
echo "  ✓ External tools configuration (${#expected_tools[@]} tools)"
echo "  ✓ Live templates configuration (${#expected_templates[@]} templates)"
echo "  ✓ File templates with proper variables"
echo "  ✓ Installation script functionality"
echo "  ✓ Documentation completeness"