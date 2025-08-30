#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
TASKS="$ROOT_DIR/editor_integration/vscode/.vscode/tasks.json"

test -f "$TASKS" || { echo "tasks.json not found" >&2; exit 1; }

grep -q 'cpo_tools.cpo_generator' "$TASKS" || { echo "Expected cpo_tools.cpo_generator in tasks.json" >&2; exit 1; }
grep -q '"owner": "tincup"' "$TASKS" || { echo "Expected owner: tincup in tasks.json" >&2; exit 1; }
echo "VSCode tasks config OK"

