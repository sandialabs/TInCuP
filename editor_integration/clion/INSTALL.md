# CLion Integration Installation Guide

This guide covers installing TInCuP integration for CLion on all supported platforms.

## Prerequisites

1. **CLion 2020.3 or later**
2. **Python 3.8+** with TInCuP tools installed:
   ```bash
   cd /path/to/tincup
   pip install -e .
   ```
3. **C++20 compatible compiler** (GCC, Clang, or MSVC)

## Quick Installation

### Option 1: Automatic Installation (Recommended)

```bash
cd /path/to/tincup
./editor_integration/clion/install.sh
```

The installer will:
- Auto-detect your CLion configuration directory
- Install external tools, live templates, and file templates
- Provide clear instructions for next steps

### Option 2: Manual Installation

If the automatic installer doesn't work:

#### 1. Install External Tools
- Open CLion
- File → Settings (Ctrl+Alt+S)
- Tools → External Tools
- Click the gear icon → Import
- Select `editor_integration/clion/external_tools.xml`
- Click OK

#### 2. Install Live Templates
- In Settings: Editor → Live Templates
- Click the gear icon → Import Settings
- Select `editor_integration/clion/live_templates.xml`
- Click OK

#### 3. Install File Templates (Optional)
- Locate your CLion config directory:
  - **Linux**: `~/.config/JetBrains/CLion<version>/`
  - **macOS**: `~/Library/Application Support/JetBrains/CLion<version>/`
  - **Windows**: `%APPDATA%\JetBrains\CLion<version>\`
- Copy `file_templates/*.hpp` and `file_templates/*.cpp` to:
  `<config_dir>/fileTemplates/includes/`

### Option 3: Custom Installation Path

```bash
./editor_integration/clion/install.sh --config-dir /custom/clion/path
```

## Platform-Specific Notes

### Linux
- CLion config typically at: `~/.config/JetBrains/CLion<version>/`
- Ensure `cpo-generator` is in your PATH
- May need to install via system package manager or pip3

### macOS
- CLion config typically at: `~/Library/Application Support/JetBrains/CLion<version>/`
- Install with Homebrew Python or system Python3
- Ensure Xcode command line tools are installed

### Windows
- CLion config typically at: `%APPDATA%\JetBrains\CLion<version>\`
- Install Python from python.org or Microsoft Store
- Ensure Python Scripts directory is in PATH

## Verification

After installation:

1. **Restart CLion**
2. **Test External Tools**:
   - Tools → External Tools → TInCuP CPO Generator → Show LLM Help
   - Should show available operation patterns
3. **Test Live Templates**:
   - Open a .cpp/.hpp file
   - Type `cpo-include` + Tab
   - Should insert TInCuP include statement
4. **Test Integration**:
   - Tools → External Tools → TInCuP CPO Generator → Generate CPO Interactive
   - Enter name and arguments when prompted

## Troubleshooting

### "cpo-generator not found"
```bash
# Check if installed
which cpo-generator

# If not found, install TInCuP tools
cd /path/to/tincup
pip install -e .

# On Windows, check PATH includes Python Scripts
echo %PATH%
```

### "No such file or directory" during installation
```bash
# Verify TInCuP directory structure
ls -la editor_integration/clion/

# Try specifying config directory manually
find ~ -name "CLion*" -type d 2>/dev/null
./install.sh --config-dir /path/to/found/clion/config
```

### External Tools not appearing
- Restart CLion completely
- Check File → Settings → Tools → External Tools
- Look for "TInCuP CPO Generator" group
- Re-import if necessary

### Live Templates not working
- File → Settings → Editor → Live Templates
- Expand "TInCuP CPO" group
- Ensure templates are enabled (checkboxes checked)
- Try typing abbreviation + Tab in appropriate context

### Templates not appearing in correct context
- Live templates are configured for C++ files
- Ensure you're in a .cpp or .hpp file
- Check template context in settings if needed

## Uninstallation

To remove TInCuP integration:

1. **External Tools**: File → Settings → Tools → External Tools → Delete "TInCuP CPO Generator" group
2. **Live Templates**: File → Settings → Editor → Live Templates → Delete "TInCuP CPO" group  
3. **File Templates**: Remove files from `<config_dir>/fileTemplates/includes/CPO_*`

## Advanced Configuration

### Custom Keyboard Shortcuts
1. File → Settings → Keymap
2. Search "External Tools" 
3. Expand "TInCuP CPO Generator"
4. Right-click tool → Add Keyboard Shortcut

Recommended shortcuts:
- **Ctrl+Alt+G**: Generate CPO with Doxygen
- **Ctrl+Alt+V**: Verify CPO Patterns
- **Ctrl+Alt+T**: Run CPO Tests

### Customize External Tools
1. File → Settings → Tools → External Tools
2. Select a TInCuP tool
3. Click Edit (pencil icon)
4. Modify parameters, working directory, etc.

### Project-Specific Configuration
- External tools use `$ProjectFileDir$` as working directory
- Ensure TInCuP is accessible from your project root
- Consider adding to project requirements or setup scripts

## Getting Help

- **Documentation**: See `editor_integration/clion/README.md`
- **Examples**: Browse live templates for usage patterns
- **Issues**: Check GitHub issues for known problems
- **Generator Help**: Run `cpo-generator --help` for CLI documentation