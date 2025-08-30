# VSCode Integration

Complete VSCode integration for `tincup` development.

## Quick Setup

1. **Copy configuration to your workspace**:
   ```bash
   cp -r editor_integration/vscode/.vscode /path/to/your/workspace/
   ```

2. **Install recommended extensions** (VSCode will prompt automatically)

3. **Install development tools**:
   ```bash
   cd /path/to/your/workspace
   make -f build_systems/make/Makefile install-dev
   ```

## What's Included

- **Tasks**: Generate CPOs, verify patterns, run tests
- **Snippets**: Quick CPO JSON specs and C++ templates  
- **Keybindings**: `Ctrl+Shift+C` + shortcuts for common operations
- **Debug configs**: Step through generator and verification logic
- **Test integration**: Run and debug tests from Test Explorer
- **Problem matchers**: CPO violations appear in Problems panel

## Key Features

| Shortcut | Action |
|----------|--------|
| `Ctrl+Shift+C` `G` | Generate CPO interactively |
| `Ctrl+Shift+C` `V` | Verify current file |
| `Ctrl+Shift+C` `T` | Run all tests |

Type `cpo-` + Tab for code snippets.

For complete documentation, see `../../docs/VSCODE_INTEGRATION.md`.