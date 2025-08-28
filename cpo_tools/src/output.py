# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

from __future__ import annotations

from pathlib import Path
from typing import Optional
import shutil
import subprocess
import sys


def wrap_output(code: str, ns: Optional[str], with_include: bool) -> str:
    """Optionally wrap generated code with include, pragma once, and namespace."""
    lines = ["#pragma once"]
    if with_include:
        lines.append("#include <tincup/tincup.hpp>")
    lines.append("")
    if ns:
        lines.append(f"namespace {ns} {{")
        lines.append("")
        lines.append(code)
        lines.append("")
        lines.append(f"}} // namespace {ns}")
        return "\n".join(lines)
    else:
        lines.append(code)
        return "\n".join(lines)


def write_output(out_path, generated_code, append, format_code, clang_format_path):
    """Write the generated code to a file or stdout."""
    if out_path:
        path = Path(out_path)
        path.parent.mkdir(parents=True, exist_ok=True)
        if append and path.exists():
            existing = path.read_text()
            sep = "\n" if not existing.endswith("\n") else ""
            path.write_text(existing + sep + generated_code)
        else:
            path.write_text(generated_code)

        if format_code:
            cf = clang_format_path or shutil.which("clang-format")
            if cf:
                try:
                    subprocess.run([cf, "-i", str(path)], check=True)
                except Exception as e:
                    print(f"Warning: clang-format failed: {e}", file=sys.stderr)
            else:
                print(
                    "Warning: clang-format not found; skipping formatting",
                    file=sys.stderr,
                )
    else:
        print(generated_code)
