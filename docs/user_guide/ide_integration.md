# Editor/IDE Integration

This document provides guidance on integrating TInCuP with various editors and IDEs.

## VSCode
See [docs/user_guide/vscode_integration.md](vscode_integration.md) for tasks, snippets, and integration tips.

## Vim

This repository contains a fully functional Vim plugin that integrates the code generator.

### Installation

Using a plugin manager like [vim-plug](https://github.com/junegunn/vim-plug), add the following to your `.vimrc`:

```vim
Plug 'sandialabs/TInCuP', {'rtp': 'editor_integration/vim'}
```

### Usage

The plugin provides two commands:

*   `:CPO <name> [args...]`: Generates a CPO.
*   `:CPOD <name> [args...]`: Generates a CPO with a Doxygen comment stub.

Example:

```vim
:CPO my_cpo $V&:x $const S&:y
```

### Vim Installer (Optional)

To help with setup, a helper script prints or appends the correct vimrc snippet for your plugin manager (vim-plug, Vundle, or Pathogen):

```bash
# Print recommended lines based on your ~/.vimrc
editor_integration/vim/install.sh

# Append to ~/.vimrc with a timestamped backup
editor_integration/vim/install.sh --apply

# Force a specific manager
editor_integration/vim/install.sh --manager plug   # or vundle | pathogen
```

Notes:
- For vim-plug, the script uses `{'rtp': 'editor_integration/vim'}`.
- For Vundle/Pathogen, it adds a `set rtp+=.../editor_integration/vim` line so Vim sees the plugin inside the repo.
- Without a plugin manager, use native packages: symlink `editor_integration/vim/{plugin,autoload}` into `~/.vim/pack/tincup/start/tincup/`.

## CLion
Full CLion integration with external tools, live templates, and file templates. See `editor_integration/clion/README.md` for setup and usage.
