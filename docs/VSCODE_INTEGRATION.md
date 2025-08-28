# VSCode Integration Guide

This document describes how to use the `tincup` tools effectively within VSCode.

## Quick Setup

1. **Open the workspace** in VSCode
2. **Install recommended extensions** (VSCode will prompt you)
3. **Run development setup**: Press `Ctrl+Shift+P`, type "Tasks: Run Task", select "Install CPO Tools"

## Key Features

### 🎯 **Keyboard Shortcuts**

| Shortcut | Action |
|----------|--------|
| `Ctrl+Shift+C` `Ctrl+G` | Generate CPO (Interactive) |
| `Ctrl+Shift+C` `Ctrl+V` | Verify Current File |
| `Ctrl+Shift+C` `Ctrl+T` | Run CPO Tests |
| `Ctrl+Shift+C` `Ctrl+S` | Generate from Selection |
| `Ctrl+Shift+C` `Ctrl+L` | Show LLM Patterns |
| `Ctrl+Shift+C` `Ctrl+A` | Verify All Files |

### 📝 **Code Snippets**

Type these prefixes and press `Tab` to expand:

**JSON Specifications:**
- `cpo-gb` → Generic binary CPO spec
- `cpo-gu` → Generic unary CPO spec  
- `cpo-fwd` → Forwarding reference CPO spec
- `cpo-var` → Variadic CPO spec
- `cpo-con` → Concrete type CPO spec
- `cpo-llm` → LLM mode semantic spec

**C++ Implementation:**
- `cpo-impl` → Manual CPO implementation template
- `tag-invoke` → Tag invoke implementation template

**Command Line:**
- `cpo-cmd` → CPO generator command

### 🔧 **Tasks (Ctrl+Shift+P → "Tasks: Run Task")**

**Generation Tasks:**
- **Generate CPO** - Generate from command line input
- **Generate CPO (Interactive)** - Prompted input
- **Generate CPO (LLM Mode)** - Show semantic patterns
- **Generate CPO from Selection** - Use selected JSON

**Verification Tasks:**
- **Verify CPO Patterns** - Check entire workspace
- **Verify Current File** - Check active file only
- **Run CPO Tests** - Run full test suite

**Setup Tasks:**
- **Install CPO Tools** - Set up development environment

## Usage Workflows

### 🚀 **Quick CPO Generation**

1. **Using Snippets** (Recommended):
   ```json
   // Type "cpo-gb" + Tab, then fill in the blanks
   {
     "cpo_name": "my_binary_op",
     "args": ["$T&: target", "$const U&: source"]
   }
   ```

2. **Using Interactive Generation**:
   - Press `Ctrl+Shift+C` `Ctrl+G`
   - Enter your JSON specification
   - Generated CPO appears in terminal

3. **Using Selection**:
   - Select JSON specification in editor
   - Press `Ctrl+Shift+C` `Ctrl+S`
   - Generated CPO appears in terminal

### 🔍 **Pattern Verification**

1. **Single File Verification**:
   - Open any `.hpp` file with CPOs
   - Press `Ctrl+Shift+C` `Ctrl+V`
   - Issues appear in Problems panel

2. **Workspace Verification**:
   - Press `Ctrl+Shift+C` `Ctrl+A`
   - All CPO issues reported in Problems panel

### 🧪 **Testing Integration**

1. **Run All Tests**:
   - Press `Ctrl+Shift+C` `Ctrl+T`
   - Or use Test Explorer in sidebar

2. **Debug Tests**:
   - Press `F5` → Select "Run CPO Tests"
   - Or debug specific tests with "Debug Specific Test"

### 🐛 **Debugging CPO Tools**

1. **Debug Generator**:
   - Press `F5` → Select "Debug CPO Generator"
   - Enter JSON specification when prompted

2. **Debug Verification**:
   - Press `F5` → Select "Debug CPO Verification"
   - Current file will be analyzed

## Examples

### Example 1: Generate Generic Binary CPO

1. Create new `.json` file or work in existing file
2. Type `cpo-gb` + `Tab`
3. Fill in:
   ```json
   {
     "cpo_name": "add_in_place",
     "args": ["$Vector&: target", "$const Vector&: source"]
   }
   ```
4. Select the JSON, press `Ctrl+Shift+C` `Ctrl+S`

### Example 2: Generate with LLM Mode

1. Type `cpo-llm` + `Tab`
2. Fill in:
   ```json
   {
     "cpo_name": "process_data", 
     "operation_type": "mutating_binary",
     "doxygen": true
   }
   ```
3. Generate using selection or interactive mode

### Example 3: Verify CPO Compliance

1. Open file with existing CPO
2. Press `Ctrl+Shift+C` `Ctrl+V`
3. Check Problems panel for any issues
4. Fix issues and re-verify

## Problem Matchers

VSCode automatically detects and highlights:
- Missing or incorrect CPO tag macro usage
- Inconsistent template parameters
- Missing `noexcept` specifications
- Incorrect concept aliases
- Naming convention violations

## Test Integration

The Python test suite is fully integrated:

1. **Test Explorer**: View and run tests from sidebar
2. **Test Output**: See results in Terminal/Output panels
3. **Debugging**: Set breakpoints and debug test cases
4. **Coverage**: Run with coverage reporting

## Configuration

All settings are preconfigured in `.vscode/settings.json`:
- C++20 standard enabled
- Python unittest integration
- File associations for templates
- Intelligent code completion

## Troubleshooting

**Common Issues:**

1. **"Module not found" errors**:
   - Run "Install CPO Tools" task
   - Ensure Python path is correct in settings

2. **Tasks not appearing**:
   - Reload VSCode window (`Ctrl+Shift+P` → "Developer: Reload Window")

3. **Snippets not working**:
   - Check file type associations
   - Ensure `editor.tabCompletion` is enabled

4. **Tests not discovered**:
   - Check Python interpreter setting
   - Verify `cpo_tools` module is importable

For more help, see the main README.md or run the "Generate CPO (LLM Mode)" task to see available patterns.
