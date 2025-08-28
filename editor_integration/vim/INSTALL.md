# tincup Vim/Neovim Integration

This plugin exposes `:CPO` and `:CPOD` to generate CPOs from within Vim/Neovim.

## Quick Options

1) Helper script (prints or appends config)

```bash
# Print recommended lines based on your vimrc/init.vim
editor_integration/vim/install.sh

# Append to vimrc/init.vim with a dated backup
editor_integration/vim/install.sh --apply

# Force a specific manager
editor_integration/vim/install.sh --manager plug   # or vundle | pathogen | packer | lazy

# Use Neovim explicitly (uses ~/.config/nvim/init.vim by default)
editor_integration/vim/install.sh --nvim
```

2) Manual installation snippets

### vim-plug (Vim/Neovim)

```vim
Plug 'sandialabs/TInCuP', {'rtp': 'editor_integration/vim'}
```

### Vundle (Vim)

```vim
Plugin 'sandialabs/TInCuP'
" Ensure Vim sees the plugin under the repo's subdirectory
execute 'set rtp+=$HOME/.vim/bundle/tincup/editor_integration/vim'
```

### Pathogen (Vim)

```vim
" Assuming you clone to ~/.vim/bundle/tincup
execute 'set rtp+=$HOME/.vim/bundle/tincup/editor_integration/vim'
```

### Native packages (Vim 8/Neovim)

```bash
mkdir -p ~/.vim/pack/tincup/start/tincup
ln -s /path/to/repo/editor_integration/vim/* ~/.vim/pack/tincup/start/tincup/
```

### packer.nvim (Neovim)

```lua
-- inside packer.startup(function(use) ... end)
use { 'sandialabs/TInCuP', rtp = 'editor_integration/vim' }
```

### lazy.nvim (Neovim)

```lua
-- lazy.nvim doesn’t support rtp= directly; append runtimepath for the plugin
require('lazy').setup({
  -- your other plugins...
})
vim.opt.rtp:append(vim.fn.stdpath('data') .. '/lazy/tincup/editor_integration/vim')
```

## Commands

- `:CPO <name> [args...]`
- `:CPOD <name> [args...]`

Examples:

```vim
:CPO my_cpo $T&:target $const U&:source
:CPOD add_in_place $V&:x $const V&:y
```

## Requirements

- Python 3.8+
- `cpo_tools` installed (from repo root): `pip install -e .`

