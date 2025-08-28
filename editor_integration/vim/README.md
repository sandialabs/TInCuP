# Vim Integration

Vim plugin for `tincup` with `:CPO` and `:CPOD` commands.

## Installation

Using [vim-plug](https://github.com/junegunn/vim-plug):

```vim
Plug 'sandialabs/TInCuP', {'rtp': 'editor_integration/vim'}
```

Using [Vundle](https://github.com/VundleVim/Vundle.vim):

```vim
Plugin 'sandialabs/TInCuP', {'rtp': 'editor_integration/vim'}
```

## Usage

The plugin provides two commands:

### `:CPO <name> [args...]`

Generate a CPO without Doxygen comments:

```vim
:CPO my_cpo $T&:target $const U&:source

Note: The runtime path now contains plugin/ and autoload/ directly under `editor_integration/vim`, so no additional subdirectory is needed.
```

### `:CPOD <name> [args...]`  

Generate a CPO with Doxygen comment stubs:

```vim
:CPOD my_binary_op $Vector&:lhs $const Vector&:rhs
```

## Examples

```vim
" Generic unary CPO
:CPO normalize $const V&:vec

" Concrete binary CPO  
:CPO add_ints int&:target const int&:source

" Forwarding reference CPO
:CPOD forward_call $T&&:callable $Args&&...:args
```

## Requirements

- Python 3.8+
- `cpo_tools` module installed (run `pip install -e .` from project root)

See also: `INSTALL.md` for Vim/Neovim installation variants and a helper installer script.
