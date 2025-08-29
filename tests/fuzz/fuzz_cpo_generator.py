#!/usr/bin/env python3
# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

# Lightweight fuzz harness for the CPO generator processing pipeline.
# Exercises JSON spec parsing + template generation to uncover unexpected exceptions.

import sys
import json

try:
    import atheris  # type: ignore
except Exception:  # pragma: no cover - atheris only in CI fuzz job
    atheris = None

from jinja2 import Environment, PackageLoader

from cpo_tools.src.process import process_input


def _process(spec: dict) -> None:
    """Try to process a fuzzed spec through the generator pipeline."""
    env = Environment(
        loader=PackageLoader("cpo_tools", "templates"),
        trim_blocks=True,
        lstrip_blocks=True,
    )
    # We expect ValueError/KeyError/TypeError for malformed inputs; those are fine.
    try:
        process_input(spec, doxygen=False, template_env=env)
    except (ValueError, KeyError, TypeError):
        return


def TestOneInput(data: bytes) -> None:  # libFuzzer entrypoint
    try:
        # Decode fuzz bytes to a string and attempt JSON parsing
        s = data.decode("utf-8", "ignore")
        obj = json.loads(s)
    except json.JSONDecodeError:
        return
    # We only care about dict-like specs
    if not isinstance(obj, dict):
        return
    # Optionally coerce minimum shape to increase code coverage
    if "cpo_name" not in obj:
        obj["cpo_name"] = "fuzz_cpo"
    _process(obj)


def main():
    if atheris is None:
        print(
            "atheris is not installed; run via CI or install atheris to fuzz locally",
            file=sys.stderr,
        )
        return 0
    atheris.Setup(sys.argv, TestOneInput)
    atheris.Fuzz()
    return 0


if __name__ == "__main__":
    sys.exit(main())

