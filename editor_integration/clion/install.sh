#!/bin/bash
# CLion TInCuP Integration Installer
# This script helps install TInCuP external tools, templates, and configurations

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CLION_CONFIG_DIR=""

# Function to find CLion configuration directory
find_clion_config() {
    local possible_paths=(
        "$HOME/.config/JetBrains/CLion"*
        "$HOME/Library/Application Support/JetBrains/CLion"*
        "$HOME/AppData/Roaming/JetBrains/CLion"*
        "$APPDATA/JetBrains/CLion"*
    )
    
    for path_pattern in "${possible_paths[@]}"; do
        if ls -d $path_pattern 2>/dev/null | head -1; then
            return 0
        fi
    done
    return 1
}

# Function to show usage
usage() {
    cat << EOF
CLion TInCuP Integration Installer

Usage: $0 [options]

Options:
    --config-dir PATH    Specify CLion configuration directory manually
    --dry-run           Show what would be installed without making changes
    --help              Show this help message

This script installs:
    - External Tools for CPO generation and verification
    - Live Templates for quick CPO scaffolding
    - File Templates for new CPO files

If no config directory is specified, the script will attempt to auto-detect
the CLion configuration directory.

Examples:
    $0                                          # Auto-detect and install
    $0 --config-dir ~/.config/JetBrains/CLion2023.3
    $0 --dry-run                               # Preview installation
EOF
}

# Parse command line arguments
DRY_RUN=false
while [[ $# -gt 0 ]]; do
    case $1 in
        --config-dir)
            CLION_CONFIG_DIR="$2"
            shift 2
            ;;
        --dry-run)
            DRY_RUN=true
            shift
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Find CLion config directory if not specified
if [[ -z "$CLION_CONFIG_DIR" ]]; then
    echo "Auto-detecting CLion configuration directory..."
    CLION_CONFIG_DIR=$(find_clion_config)
    if [[ $? -ne 0 || -z "$CLION_CONFIG_DIR" ]]; then
        echo "Could not auto-detect CLion configuration directory."
        echo "Please specify it manually with --config-dir"
        exit 1
    fi
fi

echo "Using CLion configuration directory: $CLION_CONFIG_DIR"

if [[ ! -d "$CLION_CONFIG_DIR" ]]; then
    echo "Error: CLion configuration directory does not exist: $CLION_CONFIG_DIR"
    exit 1
fi

# Check if cpo-generator is available
if ! command -v cpo-generator &> /dev/null; then
    echo "Warning: cpo-generator not found in PATH"
    echo "Please install TInCuP tools with: pip install -e /path/to/tincup"
fi

# Function to install file
install_file() {
    local src="$1"
    local dest="$2"
    local desc="$3"
    
    if [[ "$DRY_RUN" == "true" ]]; then
        echo "[DRY RUN] Would install $desc: $src -> $dest"
        return
    fi
    
    echo "Installing $desc..."
    mkdir -p "$(dirname "$dest")"
    cp "$src" "$dest"
    echo "  -> $dest"
}

# Install external tools
install_file \
    "$SCRIPT_DIR/external_tools.xml" \
    "$CLION_CONFIG_DIR/tools/external_tools.xml" \
    "External Tools"

# Install live templates  
install_file \
    "$SCRIPT_DIR/live_templates.xml" \
    "$CLION_CONFIG_DIR/templates/TInCuP_CPO.xml" \
    "Live Templates"

# Install file templates
install_file \
    "$SCRIPT_DIR/file_templates/CPO_Header.hpp" \
    "$CLION_CONFIG_DIR/fileTemplates/includes/CPO_Header.hpp" \
    "CPO Header Template"

install_file \
    "$SCRIPT_DIR/file_templates/CPO_Implementation.cpp" \
    "$CLION_CONFIG_DIR/fileTemplates/includes/CPO_Implementation.cpp" \
    "CPO Implementation Template"

if [[ "$DRY_RUN" == "true" ]]; then
    echo ""
    echo "[DRY RUN] Installation preview completed."
    echo "Run without --dry-run to perform actual installation."
else
    echo ""
    echo "Installation completed successfully!"
    echo ""
    echo "To use the integration:"
    echo "1. Restart CLion"
    echo "2. Use Tools -> External Tools -> TInCuP CPO Generator"
    echo "3. Use live templates by typing 'cpo-' and pressing Tab"
    echo "4. Create new files with CPO templates from File -> New"
    echo ""
    echo "Note: Make sure 'cpo-generator' is available in your PATH"
    echo "Install with: pip install -e /path/to/tincup"
fi