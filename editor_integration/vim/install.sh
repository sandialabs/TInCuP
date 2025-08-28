#!/usr/bin/env bash
set -euo pipefail

# tincup Vim integration installer
# - Detects vim-plug, Vundle, or Pathogen usage
# - Prints recommended vimrc lines, or appends them with --apply

REPO_SLUG="sandialabs/TInCuP"
RTP_SUBDIR="editor_integration/vim"
VIMRC="${VIMRC:-$HOME/.vimrc}"
NVIMRC_DEFAULT="$HOME/.config/nvim/init.vim"

usage() {
  cat << EOF
Usage: $(basename "$0") [--apply] [--manager plug|vundle|pathogen|packer|lazy] [--nvim] [--rc PATH]

Detects your Vim plugin manager and prints the correct configuration lines.
Use --apply to append them to your vimrc with a dated backup.

Environment:
  VIMRC      Path to your vimrc (default: ~/.vimrc)
  NVIMRC     Path to your Neovim init.vim (default: ~/.config/nvim/init.vim)
EOF
}

manager=""
apply="0"
nvim="0"
rc_override=""
while [[ $# -gt 0 ]]; do
  case "$1" in
    --apply) apply="1"; shift ;;
    --manager) manager="${2:-}"; shift 2 ;;
    --nvim) nvim="1"; shift ;;
    --rc) rc_override="${2:-}"; shift 2 ;;
    -h|--help) usage; exit 0 ;;
    *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
  esac
done

detect_manager() {
  [[ -n "$manager" ]] && { echo "$manager"; return; }
  local rc_file="$1"
  if grep -qE 'plug#begin|^\s*Plug\s' "$rc_file" 2>/dev/null; then
    echo plug; return
  fi
  if grep -qE 'vundle#begin|^\s*Plugin\s' "$rc_file" 2>/dev/null; then
    echo vundle; return
  fi
  if grep -qE 'pathogen#infect' "$rc_file" 2>/dev/null; then
    echo pathogen; return
  fi
  # Detect Neovim Lua managers (print-only mode)
  if grep -qE 'packer\.startup' "$rc_file" 2>/dev/null; then
    echo packer; return
  fi
  if grep -qE 'require\(["\'\"]lazy["\'\"]\)' "$rc_file" 2>/dev/null; then
    echo lazy; return
  fi
  echo unknown
}

print_block() {
  local mgr="$1"
  case "$mgr" in
    plug)
      cat << EOP
" --- tincup integration (vim-plug) ---
Plug '$REPO_SLUG', {'rtp': '$RTP_SUBDIR'}
" --- end tincup ---
EOP
      ;;
    vundle)
      cat << EOV
" --- tincup integration (Vundle) ---
Plugin '$REPO_SLUG'
" Ensure Vim sees the plugin under the repo's subdirectory
execute 'set rtp+=$HOME/.vim/bundle/tincup/$RTP_SUBDIR'
" --- end tincup ---
EOV
      ;;
    pathogen)
      cat << EOT
" --- tincup integration (Pathogen) ---
" Pathogen loads repos from ~/.vim/bundle; ensure runtimepath includes subdir
" If you clone repo as ~/.vim/bundle/tincup, add:
execute 'set rtp+=$HOME/.vim/bundle/tincup/$RTP_SUBDIR'
" --- end tincup ---
EOT
      ;;
    packer)
      cat << EOPK
-- --- tincup integration (packer.nvim, init.lua) ---
-- Add inside your packer.startup() use() block:
use { '$REPO_SLUG', rtp = '$RTP_SUBDIR' }
-- --- end tincup ---
EOPK
      ;;
    lazy)
      cat << EOLZ
-- --- tincup integration (lazy.nvim, init.lua) ---
-- Lazy doesn’t support rtp= directly; append runtimepath for the plugin:
require('lazy').setup({
  -- your other plugins...
})
vim.opt.rtp:append(vim.fn.stdpath('data') .. '/lazy/tincup/$RTP_SUBDIR')
-- --- end tincup ---
EOLZ
      ;;
    *)
      cat << EOU
" --- tincup integration (no manager detected) ---
" Option A: Native packages (recommended on Vim 8/Neovim):
"   mkdir -p ~/.vim/pack/tincup/start/tincup
"   ln -s /path/to/repo/$RTP_SUBDIR/* ~/.vim/pack/tincup/start/tincup/
" Option B: vim-plug:
"   Plug '$REPO_SLUG', {'rtp': '$RTP_SUBDIR'}
" --- end tincup ---
EOU
      ;;
  esac
}

append_block() {
  local block="$1"
  local bak="$VIMRC.tincup.$(date +%Y%m%d%H%M%S).bak"
  cp "$VIMRC" "$bak" || true
  printf "\n%s\n" "$block" >> "$VIMRC"
  echo "Appended configuration to $VIMRC (backup at $bak)"
}

rc_target="$VIMRC"
if [[ "$nvim" == "1" || -f "$NVIMRC_DEFAULT" ]]; then
  rc_target="${rc_override:-$NVIMRC_DEFAULT}"
fi

mgr="$(detect_manager "$rc_target")"
block="$(print_block "$mgr")"

echo "Detected rc: $rc_target"
echo "Detected manager: $mgr"
echo
echo "$block"

if [[ "$apply" == "1" ]]; then
  if [[ "$mgr" == "packer" || "$mgr" == "lazy" ]]; then
    echo "Refusing to auto-modify Lua configs for $mgr. Please copy the snippet above into $rc_target." >&2
    exit 1
  fi
  if [[ ! -f "$rc_target" ]]; then
    echo "Creating $rc_target"
    mkdir -p "$(dirname "$rc_target")"
    touch "$rc_target"
  fi
  append_block "$block"
  echo "Done. Restart Vim/Neovim or :source $rc_target"
else
  echo "(Dry run) To append these lines automatically, re-run with --apply"
fi
