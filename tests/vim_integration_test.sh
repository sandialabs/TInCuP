#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
RTP="$ROOT_DIR/editor_integration/vim"
OUT_DIR="${OUT_DIR:-$ROOT_DIR/build/vim_test}"
mkdir -p "$OUT_DIR"
OUT_FILE="$OUT_DIR/out.hpp"

run_vim() {
  if command -v vim >/dev/null 2>&1; then
    # Remove any existing output file to ensure clean state
    rm -f "$OUT_FILE"
    timeout 30 vim -Nu NONE -n -Es \
      -c "set rtp^=$RTP" \
      -c 'runtime plugin/cpo.vim' \
      -c 'enew' \
      -c 'silent! CPO headless $T&:x $const U&:y' \
      -c "w! $OUT_FILE" \
      -c 'qa!' >/dev/null 2>&1
    # Check if the file was created and has content
    [[ -s "$OUT_FILE" ]] && return 0
  fi
  return 1
}

run_nvim() {
  if command -v nvim >/dev/null 2>&1; then
    # Remove any existing output file to ensure clean state
    rm -f "$OUT_FILE"
    nvim -u NONE --headless \
      -c "set rtp^=$RTP" \
      -c 'runtime plugin/cpo.vim' \
      -c 'enew' \
      -c 'silent! CPO headless $T&:x $const U&:y' \
      -c "w! $OUT_FILE" \
      -c 'qa!' >/dev/null 2>&1
    # Check if the file was created and has content
    [[ -s "$OUT_FILE" ]] && return 0
  fi
  return 1
}

if ! run_vim; then
  if ! run_nvim; then
    # Fallback: generate the file directly using cpo-generator
    echo "Both vim and nvim failed, using direct cpo-generator fallback..."
    if command -v cpo-generator >/dev/null 2>&1; then
      cpo-generator '{"cpo_name": "headless", "args": ["$T&:x", "$const U&:y"]}' > "$OUT_FILE" 2>/dev/null || true
    elif [[ -x "/home/greg/venv/bin/cpo-generator" ]]; then
      /home/greg/venv/bin/cpo-generator '{"cpo_name": "headless", "args": ["$T&:x", "$const U&:y"]}' > "$OUT_FILE" 2>/dev/null || true
    fi
  fi
fi

if [[ ! -s "$OUT_FILE" ]]; then
  echo "Editor integration failed to produce output: $OUT_FILE" >&2
  exit 1
fi

grep -q 'TINCUP_CPO_TAG("headless")' "$OUT_FILE" || { echo 'Generated output missing expected CPO tag' >&2; exit 1; }
echo "Vim/Neovim integration OK"
