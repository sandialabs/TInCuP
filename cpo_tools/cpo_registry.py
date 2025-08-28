#!/usr/bin/env python3
# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

"""
Scan headers for TINCUP CPO tags and emit a registry in JSON and Markdown.

Finds occurrences of TINCUP_CPO_TAG("name") or legacy CPO_TAG("name").

Usage:
  python3 cpo_tools/cpo_registry.py --root include --out docs
"""
from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import List, Optional


TAG_RE = re.compile(r"\b(TINCUP_CPO_TAG|CPO_TAG)\s*\(\s*\"([^\"]+)\"\s*\)")
STRUCT_RE = re.compile(r"\bstruct\s+([a-zA-Z_][a-zA-Z0-9_]*)\b")


@dataclass
class CPOEntry:
    name: str
    qualified: str
    header: str
    struct: Optional[str]
    line: int


def scan_file(path: Path) -> List[CPOEntry]:
    entries: List[CPOEntry] = []
    try:
        text = path.read_text(encoding="utf-8", errors="ignore")
    except Exception:
        return entries

    struct_name = None
    # Track last seen struct to associate with the macro placement
    for i, line in enumerate(text.splitlines(), start=1):
        s = STRUCT_RE.search(line)
        if s:
            struct_name = s.group(1)

        m = TAG_RE.search(line)
        if m:
            tag_name = m.group(2)
            entries.append(
                CPOEntry(
                    name=tag_name,
                    qualified=f"tincup::{tag_name}",
                    header=str(path),
                    struct=struct_name,
                    line=i,
                )
            )
    return entries


def scan_root(root: Path) -> List[CPOEntry]:
    patterns = ("*.hpp", "*.hh", "*.h")
    files = []
    for pat in patterns:
        files.extend(root.rglob(pat))
    all_entries: List[CPOEntry] = []
    for f in files:
        all_entries.extend(scan_file(f))
    # De-duplicate by (name, header, line)
    seen = set()
    uniq: List[CPOEntry] = []
    for e in all_entries:
        key = (e.name, e.header, e.line)
        if key in seen:
            continue
        seen.add(key)
        uniq.append(e)
    # Sort by name then header
    uniq.sort(key=lambda x: (x.name, x.header, x.line))
    return uniq


def write_outputs(entries: List[CPOEntry], out_dir: Path) -> None:
    out_dir.mkdir(parents=True, exist_ok=True)
    # JSON
    (out_dir / "cpo_registry.json").write_text(
        json.dumps([asdict(e) for e in entries], indent=2), encoding="utf-8"
    )
    # Markdown
    md_lines = ["# TInCuP CPO Registry", "", f"Total: {len(entries)}", ""]
    for e in entries:
        loc = f"{Path(e.header)}:{e.line}"
        struct = f" (struct {e.struct})" if e.struct else ""
        md_lines.append(f"- `{e.name}`: `{e.qualified}` — {loc}{struct}")
    (out_dir / "cpo_registry.md").write_text(
        "\n".join(md_lines) + "\n", encoding="utf-8"
    )


def main() -> int:
    ap = argparse.ArgumentParser(description="Generate TInCuP CPO registry")
    ap.add_argument("--root", default="include", help="Root directory to scan")
    ap.add_argument("--out", default="docs", help="Output directory for registry files")
    args = ap.parse_args()

    root = Path(args.root).resolve()
    out = Path(args.out).resolve()
    entries = scan_root(root)
    write_outputs(entries, out)
    print(f"Found {len(entries)} CPOs. Wrote {out}/cpo_registry.json and .md")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
