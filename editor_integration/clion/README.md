# CLion Integration

Complete CLion integration for `tincup` development with external tools, live templates, and file templates.

## Quick Setup

### Automatic Installation

1. **Install TInCuP tools** (if not already done):
   ```bash
   cd /path/to/tincup
   pip install -e .
   ```

2. **Run the installer**:
   ```bash
   editor_integration/clion/install.sh
   ```

3. **Restart CLion** to load the new configuration

### Manual Installation

If you prefer manual installation or the automatic installer doesn't work:

1. **Install External Tools**:
   - File → Settings → Tools → External Tools
   - Click the gear icon → Import
   - Select `external_tools.xml`

2. **Install Live Templates**:
   - File → Settings → Editor → Live Templates  
   - Click the gear icon → Import Settings
   - Select `live_templates.xml`

3. **Install File Templates**:
   - Copy files from `file_templates/` to your CLion config directory under `fileTemplates/includes/`

## What's Included

### External Tools
Access via **Tools → External Tools → TInCuP CPO Generator**:

| Tool | Description |
|------|-------------|
| **Generate CPO** | Generate a CPO from selected text or prompt |
| **Generate CPO with Doxygen** | Generate a CPO with documentation comments |
| **Generate CPO Interactive** | Interactively specify CPO name and arguments |
| **Verify CPO Patterns** | Verify current file follows CPO patterns |
| **Run CPO Tests** | Execute the full test suite |
| **Show LLM Help** | Display available LLM operation patterns |

### Live Templates
Type the abbreviation and press **Tab**:

| Abbreviation | Description |
|--------------|-------------|
| `cpo-generic-unary` | Generic unary CPO template |
| `cpo-generic-binary` | Generic binary CPO template |
| `tag-invoke-impl` | tag_invoke implementation |
| `cpo-include` | TInCuP include with using directive |

### File Templates
Available when creating new files:

- **CPO Header** (`CPO_Header.hpp`) - Complete CPO declaration
- **CPO Implementation** (`CPO_Implementation.cpp`) - tag_invoke implementations

## Usage Examples

### Using External Tools

1. **Quick CPO Generation**:
   - Select text like `my_cpo $T&:target $const U&:source` 
   - Tools → External Tools → Generate CPO with Doxygen
   - CPO code appears in console

2. **Interactive Generation**:
   - Tools → External Tools → Generate CPO Interactive
   - Enter CPO name and arguments when prompted

3. **Pattern Verification**:
   - Open a CPO file
   - Tools → External Tools → Verify CPO Patterns
   - See compliance results in console

### Using Live Templates

1. **Create Generic Binary CPO**:
   - Type `cpo-generic-binary` + Tab
   - Fill in the template variables:
     - `NAME`: CPO name (e.g., `add_to`)
     - `ARG1`: First argument name (e.g., `target`)  
     - `ARG2`: Second argument name (e.g., `source`)

2. **Add tag_invoke Implementation**:
   - Type `tag-invoke-impl` + Tab
   - Fill in:
     - `CPO`: CPO name
     - `PARAMS`: Parameter list
     - `RETURN_TYPE`: Return type
     - `BODY`: Implementation code

### Using File Templates

1. **Create New CPO Header**:
   - File → New → C++ Header File
   - Choose "CPO Header" template
   - Fill in template variables

2. **Create Implementation File**:
   - File → New → C++ Source File
   - Choose "CPO Implementation" template

## Keyboard Shortcuts

You can assign keyboard shortcuts to external tools:

1. File → Settings → Keymap
2. Search for "External Tools"
3. Assign shortcuts to TInCuP tools

Suggested shortcuts:
- **Ctrl+Alt+G**: Generate CPO with Doxygen
- **Ctrl+Alt+V**: Verify CPO Patterns
- **Ctrl+Alt+T**: Run CPO Tests

## Integration with CMake

CLion automatically detects CMake projects. The TInCuP library integrates seamlessly:

```cmake
# In your CMakeLists.txt
find_package(tincup REQUIRED)
target_link_libraries(your_target PRIVATE tincup::tincup)
```

Or with FetchContent:
```cmake
FetchContent_Declare(tincup 
    GIT_REPOSITORY https://github.com/sandialabs/TInCuP.git)
FetchContent_MakeAvailable(tincup)
target_link_libraries(your_target PRIVATE tincup::tincup)
```

## Troubleshooting

### "cpo-generator not found"
- Ensure TInCuP is installed: `pip install -e /path/to/tincup`
- Check PATH includes the Python scripts directory

### External Tools Not Working
- Verify `cpo-generator` works from terminal
- Check Working Directory is set to `$ProjectFileDir$`
- Restart CLion after installing tools

### Live Templates Not Appearing
- Check they're imported under File → Settings → Editor → Live Templates
- Ensure context is set to C++ files
- Try typing abbreviation + Tab in a .cpp/.hpp file

## Advanced Usage

### Custom External Tool Arguments

You can modify the external tools to use different argument patterns:

1. File → Settings → Tools → External Tools
2. Select a TInCuP tool and click Edit
3. Modify the Parameters field

Example patterns:
- Simple: `{"cpo_name": "test", "args": ["$T&: arg"]}`
- LLM Mode: `{"cpo_name": "add", "operation_type": "mutating_binary"}`
- With namespace: `{"cpo_name": "test", "args": ["$T&: arg"]} --namespace mylib`

### Integration with Git Hooks

Add CPO verification to your git pre-commit hook:
```bash
# In .git/hooks/pre-commit
python3 -m cpo_tools.cpo_verification .
```

## Requirements

- CLion 2020.3 or later
- Python 3.8+
- TInCuP tools installed (`pip install -e .`)
- C++20 compatible compiler

For complete documentation, see `../../docs/` directory.
