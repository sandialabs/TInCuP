#!/usr/bin/env python3
# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

"""
Update README.md to pin installation commands to the version in VERSION.

Replaces occurrences of TInCuP==<semver> with the current version from the
top-level VERSION file. This avoids shell substitutions (e.g., $(cat VERSION))
in docs for better cross-platform clarity (e.g., Windows/VS Code users).
"""

from pathlib import Path
import re
import sys


REPO_ROOT = Path(__file__).parent.parent
README = REPO_ROOT / "README.md"
VERSION_FILE = REPO_ROOT / "VERSION"


def main() -> int:
    if not VERSION_FILE.exists():
        print("ERROR: VERSION file not found.")
        return 1
    version = VERSION_FILE.read_text(encoding="utf-8").strip()
    if not version:
        print("ERROR: VERSION file is empty.")
        return 1

    if not README.exists():
        print("ERROR: README.md not found.")
        return 1

    content = README.read_text(encoding="utf-8")

    # Apply multiple fixes in order:
    # 1) Normal case: replace TInCuP==<ver> with TInCuP=={version}
    pattern1 = re.compile(r"(TInCuP==)(\d+\.\d+\.\d+)")
    new_content = pattern1.sub(lambda m: f"{m.group(1)}{version}", content)

    # 2) Cleanup for previous bad replacement that left literal "\\g<1>" in commands
    #    e.g., "pip install \\g<1>1.2.3" -> "pip install TInCuP=={version}"
    pattern2 = re.compile(r"(pip(?:x)? install )\\g<1>\d+\.\d+\.\d+")
    new_content = pattern2.sub(lambda m: f"{m.group(1)}TInCuP=={version}", new_content)

    # If nothing changed, be explicit
    if new_content == content:
        print("README.md already up-to-date with VERSION")
        return 0

    README.write_text(new_content, encoding="utf-8")
    print(f"Updated README.md install versions to {version}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
