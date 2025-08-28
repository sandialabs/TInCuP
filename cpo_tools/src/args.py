# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

import argparse


def parse_args():
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description="CPO Generator for C++",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""Note on Vim/Compact Syntax: The 'args' array uses a shorthand where a type prefixed with '$' (e.g., '$V&') is treated as a template type name (e.g., 'typename V').

Examples for Runtime Dispatch:

Boolean Dispatch:
$ cpo-generator '{"cpo_name": "choose_path", 
                  "args": ["$T&: input"], 
                  "runtime_dispatch": {
                    "type": "bool", 
                    "dispatch_arg": "selector", 
                    "options": ["yin", "yang"]}}'
                 

String Dispatch:
$ cpo-generator '{"cpo_name": "execute_policy", 
                  "args": ["$T&: data"], 
                  "runtime_dispatch": {
                    "type": "string", 
                    "dispatch_arg": "policy_name", 
                    "options": ["fast", "safe", "debug"]}}'
""",
    )
    parser.add_argument(
        "json_spec",
        nargs="?",
        default=None,
        help="A JSON string defining the CPO. If omitted, reads from stdin.",
    )
    parser.add_argument(
        "--from",
        dest="from_path",
        metavar="FILE",
        help="Read specification from a file (JSON by default, or YAML with --format yaml or .yaml extension).",
    )
    parser.add_argument(
        "--format",
        choices=["auto", "json", "yaml"],
        default="auto",
        help="Input file format for --from. Default: auto-detect from extension.",
    )
    parser.add_argument(
        "--doxygen",
        action="store_true",
        help="Generate Doxygen comments and a tag_invoke skeleton.",
    )
    parser.add_argument(
        "--llm-help",
        action="store_true",
        help="Show a list of predefined operation types for LLM-based generation.",
    )
    parser.add_argument(
        "--emit-tag-invoke",
        dest="emit_stub",
        action="store_true",
        help="Emit a declaration-only tag_invoke stub matching the CPO signature.",
    )
    parser.add_argument(
        "--emit-trait-impl",
        dest="emit_trait_impl",
        action="store_true",
        help="Emit a tincup::cpo_impl<CPO, Target> specialization skeleton (formatter-style).",
    )
    parser.add_argument(
        "--impl-target",
        dest="impl_target",
        metavar="TYPE",
        help="Target type for trait specialization, e.g. 'Kokkos::View<...>'. Use '...' to denote a template parameter pack.",
    )
    parser.add_argument(
        "--impl-guard",
        dest="impl_guard",
        metavar="MACRO",
        help="Wrap the generated trait specialization in #ifdef MACRO/#endif.",
    )
    parser.add_argument(
        "--emit-body-guard",
        dest="emit_guard",
        metavar="MACRO",
        help="If provided with --emit-tag-invoke, also emit a stub definition wrapped in #ifdef MACRO. For non-void returns this definition intentionally fails to compile so you replace it.",
    )
    parser.add_argument(
        "--format-code",
        dest="format_code",
        action="store_true",
        help="If writing to a file, run clang-format on the result when available.",
    )
    parser.add_argument(
        "--clang-format",
        dest="clang_format",
        metavar="PATH",
        help="Path to clang-format binary. Uses PATH lookup if not provided.",
    )
    parser.add_argument(
        "--out",
        dest="out_path",
        metavar="PATH",
        help="Write output to a file instead of stdout.",
    )
    parser.add_argument(
        "--append",
        dest="append",
        action="store_true",
        help="Append to the --out file instead of overwriting.",
    )
    parser.add_argument(
        "--namespace",
        dest="namespace",
        metavar="NS",
        help="Wrap the generated CPO in a namespace.",
    )
    parser.add_argument(
        "--with-include",
        dest="with_include",
        action="store_true",
        help="Prepend #include <tincup/tincup.hpp> to the output.",
    )
    args = parser.parse_args()
    return args, parser
