#!/usr/bin/env python3
# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

"""
CPO Generator for C++

Generates C++ Customization Point Object (CPO) boilerplate code from a JSON definition.
This is a standalone tool for creating modern C++20 CPOs using the tag_invoke pattern.

The script supports two main modes of operation:

1.  VIM_MODE: A compact syntax for generating CPOs, originally designed for Vim integration.
    Example:
    cpo-generator '{"cpo_name": "my_cpo", "args": ["$T&: target", "$const U&: source"]}'

2.  LLM_MODE: A semantic, high-level interface for generating CPOs based on predefined
    operation patterns, making it more accessible for Large Language Models (LLMs) and users
    unfamiliar with the compact syntax.
    Example:
    cpo-generator '{"cpo_name": "process", "operation_type": "mutating_binary"}'

The script can read the JSON input from a command-line argument or from stdin,
making it suitable for both interactive and scripted use.
"""
import sys
import json
from pathlib import Path
from jinja2 import Environment, PackageLoader

try:
    import yaml  # type: ignore
except Exception:  # optional
    yaml = None

from cpo_tools.src.args import parse_args
from cpo_tools.src.llm import show_llm_help
from cpo_tools.src.process import process_input
from cpo_tools.src.trait_impl import render_trait_impl
from cpo_tools.src.output import wrap_output, write_output


def main():
    """Main entry point for the CPO generator script."""
    args = parse_args()

    if args.llm_help:
        show_llm_help()
        return

    # Resolve input
    input_data = None
    if args.from_path:
        path = Path(args.from_path)
        if not path.exists():
            print(f"Error: File not found: {path}", file=sys.stderr)
            sys.exit(1)
        fmt = args.format
        if fmt == "auto":
            if path.suffix.lower() in (".yml", ".yaml"):
                fmt = "yaml"
            else:
                fmt = "json"
        try:
            text = path.read_text()
            if fmt == "json":
                input_data = json.loads(text)
            elif fmt == "yaml":
                if yaml is None:
                    print(
                        "Error: PyYAML not available. Install pyyaml or use --format json.",
                        file=sys.stderr,
                    )
                    sys.exit(1)
                input_data = yaml.safe_load(text)
            else:
                print(f"Error: Unsupported format: {fmt}", file=sys.stderr)
                sys.exit(1)
        except Exception as e:
            print(f"Error reading spec from {path}: {e}", file=sys.stderr)
            sys.exit(1)
    else:
        input_str = ""
        if args.json_spec:
            input_str = args.json_spec
        elif not sys.stdin.isatty():
            input_str = sys.stdin.read()
        else:
            parser.print_help()
            sys.exit(1)
        try:
            input_data = json.loads(input_str)
        except json.JSONDecodeError:
            print(
                f"Error: Invalid JSON input provided.\n'{input_str}'", file=sys.stderr
            )
            sys.exit(1)

    try:
        template_env = Environment(
            loader=PackageLoader("cpo_tools", "templates"),
            trim_blocks=True,
            lstrip_blocks=True,
        )
        generated_code, ctx = process_input(input_data, args.doxygen, template_env)

        if args.emit_stub:
            sig = (
                f"constexpr auto tag_invoke({ctx['cpo_name']}_ftor, {ctx['arg_pairs']})"
            )
            if ctx["has_generics"]:
                sig = f"template<{ctx['arg_types']}>\n{sig}"
            stub = sig + ";\n"
            if args.emit_guard:
                guard = args.emit_guard
                returns_void = False
                llm = ctx.get("llm_metadata") or {}
                if (
                    isinstance(llm, dict)
                    and llm.get("return_constraint") == "returns_void_c"
                ):
                    returns_void = True
                body = " {\n    // TODO: implement\n}"
                if not returns_void:
                    body = (
                        " {\n"
                        "#  if defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)"
                        '    static_assert(true == false, "Provide implementation or disable guard");\n'
                        "#  endif\n"
                        "}"
                    )
                defn = sig + body + "\n"
                stub += f"\n#ifdef {guard}\n{defn}#endif\n"
            generated_code = generated_code.rstrip() + "\n\n" + stub

        # Optionally append a trait specialization skeleton
        if args.emit_trait_impl:
            if not args.impl_target:
                print("Error: --emit-trait-impl requires --impl-target TYPE (e.g., 'Kokkos::View<...>')", file=sys.stderr)
                sys.exit(1)
            trait_code = render_trait_impl(template_env, ctx["cpo_name"], args.impl_target, ctx)
            if args.impl_guard:
                trait_code = f"#ifdef {args.impl_guard}\n{trait_code}\n#endif\n"
            generated_code = generated_code.rstrip() + "\n\n" + trait_code

        if args.namespace or args.with_include:
            generated_code = wrap_output(
                generated_code, args.namespace, args.with_include
            )

        # Strip all $ and ' symbols since they're not valid in C++ and were only used for parsing
        clean_generated_code = generated_code.replace('$', '').replace("'", '')
        
        write_output(
            args.out_path,
            clean_generated_code,
            args.append,
            args.format_code,
            args.clang_format,
        )

    except (ValueError, KeyError) as e:
        print(f"Error processing input: {e}", file=sys.stderr)
        sys.exit(1)


if __name__ == "__main__":
    main()
