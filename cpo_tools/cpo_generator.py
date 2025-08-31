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

Modes
-----
1) VIM_MODE: Compact syntax for CPOs (original Vim integration)
   Example:
     cpo-generator '{"cpo_name": "my_cpo", "args": ["$T&: target", "$const U&: source"]}'

2) LLM_MODE: Semantic, high-level interface based on predefined operation patterns
   Example:
     cpo-generator '{"cpo_name": "process", "operation_type": "mutating_binary"}'

The script can read the JSON input from a command-line argument or from stdin,
making it suitable for both interactive and scripted use.

Trait-based Third-Party Integration
-----------------------------------
You can specialize a formatter-style trait for third-party/container types and optionally emit
an ADL-visible forwarding shim, keeping your core CPO code free of TPL references.

Generic syntax in --impl-target:
- Use '$T' to declare a type parameter T (template<typename T>).
- Use '$Rest...' to declare a parameter pack (template<typename... Rest>).
- Use '...' for an anonymous pack (template<typename... P> with P...).
- Bare identifiers (e.g., 'double') are concrete and do not become template parameters.
- Named packs without '$' (e.g., 'Rest...') are invalid and will error.

Examples (std::vector integration, no external deps):

1) Emit only a trait specialization (formatter-style):
   cpo-generator --from-registry add_in_place_ftor \
     --impl-target 'std::vector<$T, $Alloc>' \
     --trait-impl-only

   // Produces (simplified):
   // namespace tincup {
   // template<typename T, typename Alloc>
   // struct cpo_impl<add_in_place_ftor, std::vector<T, Alloc>> { /* static call(...) */ };
   // }

2) Emit trait + ADL-visible shim in your CPO's namespace:
   cpo-generator --from-registry add_in_place_ftor \
     --impl-target 'std::vector<$T, $Alloc>' \
     --emit-trait-impl --emit-adl-shim --shim-namespace 'myproj'

   // Adds (simplified):
   // namespace myproj {
   // template<typename T, typename Alloc, typename... Args>
   // constexpr auto tag_invoke(add_in_place_ftor, std::vector<T, Alloc>&, Args&&...);
   // }

3) Concrete head + generic tail:
   cpo-generator --from-registry add_in_place_ftor \
     --impl-target 'std::vector<double, $Rest...>' \
     --trait-impl-only

4) Without a registry (pass JSON spec directly):
   cpo-generator '{"cpo_name":"add_in_place","args":["$V&&: y","$const V&: x"]}' \
     --impl-target 'std::vector<$T, $Alloc>' --emit-trait-impl

Registry support
----------------
Use cpo_tools/cpo_registry.py to scan your headers for CPOs and emit docs/cpo_registry.json:
  python3 cpo_tools/cpo_registry.py --root include --out docs
Then pass --from-registry <tag-name or functor-struct> to refer to an existing CPO
without re-specifying the JSON.
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
from cpo_tools.src.shim_impl import render_adl_shim
from cpo_tools.src.output import wrap_output, write_output


def main():
    """Main entry point for the CPO generator script."""
    args,parser = parse_args()

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
        # Support a minimal flow when only generating a trait impl from registry
        minimal_trait_flow = (
            (args.trait_impl_only or args.emit_trait_impl or args.impl_target)
            and args.from_registry
            and not args.json_spec
            and sys.stdin.isatty()
        )
        if minimal_trait_flow:
            input_data = {"cpo_name": args.from_registry, "args": []}
        else:
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
        # If using registry-only trait generation, resolve cpo_name from registry and skip full processing
        ctx = None
        if args.from_registry and (args.trait_impl_only or args.impl_target):
            # Try to read registry
            try:
                reg_path = Path(args.registry_path)
                reg = json.loads(reg_path.read_text()) if reg_path.exists() else None
            except Exception as e:
                print(f"Warning: Failed to read registry at {reg_path}: {e}", file=sys.stderr)
                reg = None

            cpo_name = None
            if reg:
                key = args.from_registry
                # Match by struct or tag name
                for entry in reg:
                    struct = entry.get("struct")
                    name = entry.get("name")
                    if key == struct or key == name:
                        # Prefer struct-derived base (strip _ftor)
                        if struct and struct.endswith("_ftor"):
                            cpo_name = struct[:-5]
                        elif struct:
                            cpo_name = struct
                        else:
                            cpo_name = name
                        break
            # Fallback: use provided key directly
            if not cpo_name:
                cpo_name = args.from_registry
                if cpo_name.endswith("_ftor"):
                    cpo_name = cpo_name[:-5]

            ctx = {"cpo_name": cpo_name, "has_generics": False, "arg_pairs": "", "arg_types": "", "llm_metadata": {}}

        if ctx is None:
            generated_code, ctx = process_input(input_data, args.doxygen, template_env)
        else:
            generated_code = ""  # Will be replaced if not trait-only

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

        # If --impl-target provided, imply --emit-trait-impl
        if args.impl_target and not args.emit_trait_impl:
            args.emit_trait_impl = True

        # Optionally append or emit a trait specialization skeleton
        if args.emit_trait_impl:
            if not args.impl_target:
                print("Error: --emit-trait-impl requires --impl-target TYPE (e.g., 'Kokkos::View<T>' or 'Kokkos::View<...>')", file=sys.stderr)
                sys.exit(1)
            trait_code = render_trait_impl(template_env, ctx["cpo_name"], args.impl_target, ctx)
            if args.impl_guard:
                trait_code = f"#ifdef {args.impl_guard}\n{trait_code}\n#endif\n"
            if args.trait_impl_only:
                generated_code = trait_code
            else:
                generated_code = generated_code.rstrip() + "\n\n" + trait_code

        # Optionally append an ADL shim in the specified namespace
        if args.emit_adl_shim:
            if not args.impl_target:
                print("Error: --emit-adl-shim requires --impl-target TYPE to know the receiver type.", file=sys.stderr)
                sys.exit(1)
            shim_code = render_adl_shim(template_env, ctx["cpo_name"], args.impl_target, args.shim_namespace)
            generated_code = generated_code.rstrip() + "\n\n" + shim_code

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
