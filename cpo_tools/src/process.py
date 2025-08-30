# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

import re
from jinja2 import Environment


def process_semantic_constraints(llm_metadata, cpo_name, concept_types):
    """Process semantic constraints from LLM metadata into C++ concepts."""
    if not llm_metadata or "semantic_constraints" not in llm_metadata:
        return None
    
    constraints = []
    for constraint in llm_metadata["semantic_constraints"]:
        # Replace template placeholders with actual values
        processed_constraint = constraint.format(
            cpo_name=cpo_name,
            concept_types=concept_types
        )
        constraints.append(processed_constraint)
    
    # Map return constraint names to C++ type traits
    return_constraint_map = {
        "returns_void_c": "std::is_void_v",
        "returns_value_c": "!std::is_void_v", 
        "returns_new_object_c": "!std::is_void_v"
    }
    
    return_constraint = None
    if "return_constraint" in llm_metadata:
        constraint_name = llm_metadata["return_constraint"]
        return_constraint = return_constraint_map.get(constraint_name)
    
    return {
        "constraints": constraints,
        "return_constraint": return_constraint,
        "requires_const_safe": llm_metadata.get("requires_const_safe", False),
        "requires_mutable": llm_metadata.get("requires_mutable", [])
    }


def parse_cpp_type(full_type):
    """Strips qualifiers to find the base template type name."""
    base_type = re.sub(r"\bconst\b|\bvolatile\b", "", full_type)
    base_type = base_type.replace("&", "").replace("*", "").replace("...", "")
    return base_type.strip()


def process_input(input_data, generate_doxygen_cli, template_env: Environment):
    """Processes the JSON input and returns generated code plus context for stubs."""
    from .llm import detect_input_mode, process_llm_input

    input_mode = detect_input_mode(input_data)
    is_llm_mode = input_mode == "LLM_MODE"
    if is_llm_mode:
        input_data = process_llm_input(input_data)

    generate_doxygen = generate_doxygen_cli or input_data.get("doxygen", False)
    cpo_name = input_data["cpo_name"]
    raw_args = input_data["args"]
    llm_metadata = input_data.get("_llm_metadata", {})

    # --- New: Handle runtime dispatch ---
    dispatch_info = None
    if "runtime_dispatch" in input_data:
        dispatch_config = input_data["runtime_dispatch"]
        dispatch_type = dispatch_config.get("type")

        if dispatch_type == "bool":
            dispatch_arg = dispatch_config.get("dispatch_arg")
            if not dispatch_arg:
                raise ValueError(
                    "runtime_dispatch of type 'bool' requires a 'dispatch_arg' key."
                )

            options = dispatch_config.get("options", ["first_tag", "second_tag"])
            if not isinstance(options, list) or len(options) != 2:
                raise ValueError(
                    "runtime_dispatch 'options' for 'bool' type must be an array of two strings."
                )

            dispatch_info = {
                "type": "bool",
                "dispatch_arg": dispatch_arg,
                "options": options,
            }
        elif dispatch_type == "string":
            dispatch_arg = dispatch_config.get("dispatch_arg")
            if not dispatch_arg:
                raise ValueError(
                    "runtime_dispatch of type 'string' requires a 'dispatch_arg' key."
                )

            options = dispatch_config.get("options")
            if not isinstance(options, list) or len(options) < 2:
                raise ValueError(
                    "runtime_dispatch 'options' for 'string' type must be an array of at least two strings."
                )

            dispatch_info = {
                "type": "string",
                "dispatch_arg": dispatch_arg,
                "options": options,
            }

    final_arg_pairs = raw_args

    parsed_args = []
    generic_base_types = {"non_variadic": set(), "variadic": set()}
    has_variadic = False
    for pair_str in final_arg_pairs:
        try:
            full_type, name = [part.strip() for part in pair_str.rsplit(":", 1)]
            is_generic = full_type.startswith("$")
            is_variadic = "..." in full_type
            is_forwarding_ref = False

            arg_data = {"name": name, "is_variadic": is_variadic}

            if is_generic:
                type_name = full_type[1:]
                base_type = parse_cpp_type(type_name)
                arg_data["base"] = base_type

                if is_variadic:
                    generic_base_types["variadic"].add(base_type)
                else:
                    generic_base_types["non_variadic"].add(base_type)

                if type_name.strip().endswith(("&&", "&&...")):
                    is_forwarding_ref = True
                    arg_data["full"] = base_type + "&&"
                else:
                    arg_data["full"] = type_name.replace("...", "")
            else:
                arg_data["full"] = full_type.replace("...", "")

            arg_data["is_forwarding"] = is_forwarding_ref
            if is_variadic:
                has_variadic = True
            parsed_args.append(arg_data)
        except ValueError:
            raise ValueError(
                f"Invalid argument format '{pair_str}'. Expected format 'type: name'"
            )

    concept_types_list = [
        (arg["base"] if arg["is_forwarding"] else arg["full"]) for arg in parsed_args
    ]

    concept_types_list_no_dispatch = []
    dispatch_arg_name = dispatch_info["dispatch_arg"] if dispatch_info else None
    for i, arg in enumerate(parsed_args):
        if arg["name"] != dispatch_arg_name:
            concept_types_list_no_dispatch.append(concept_types_list[i])

    arg_pairs_str = ", ".join(
        f"{arg['full']}{'...' if arg['is_variadic'] else ''} {arg['name']}"
        for arg in parsed_args
    )
    arg_names_str = ", ".join(
        [
            (
                f"std::forward<{arg['base']}>({arg['name']}){'...' if arg['is_forwarding'] else ''}"
                if arg["is_forwarding"]
                else f"{arg['name']}{'...' if arg['is_variadic'] else ''}"
            )
            for arg in parsed_args
        ]
    )

    arg_pairs_no_dispatch_str = arg_pairs_str
    arg_names_no_dispatch_str = arg_names_str

    if dispatch_info and dispatch_info["type"] == "bool":
        dispatch_param = f"bool {dispatch_info['dispatch_arg']} = false"
        if arg_pairs_str:
            arg_pairs_str += f", {dispatch_param}"
        else:
            arg_pairs_str = dispatch_param

    concept_types_str = ", ".join(
        f"{t}{'...' if parsed_args[i]['is_variadic'] else ''}"
        for i, t in enumerate(concept_types_list)
    )
    
    # Create canonical concept argument string for consistency
    # This is the single source of truth for all tincup:: concept family uses
    if concept_types_str:
        canonical_concept_args = f"{cpo_name}_ftor, {concept_types_str}"
    else:
        canonical_concept_args = f"{cpo_name}_ftor"
    
    # Process semantic constraints for enhanced concepts
    semantic_info = process_semantic_constraints(llm_metadata, cpo_name, concept_types_str)
    
    template_context = {
        "cpo_name": cpo_name,
        "arg_pairs": arg_pairs_str,
        "arg_names": arg_names_str,
        "concept_types": concept_types_str,
        "canonical_concept_args": canonical_concept_args,
        "has_variadic": has_variadic,
        "semantic_info": semantic_info,
        "is_llm_mode": is_llm_mode,
        "dispatch_info": dispatch_info,
        "arg_pairs_no_dispatch": arg_pairs_no_dispatch_str,
        "arg_names_no_dispatch": arg_names_no_dispatch_str,
        "concept_types_no_dispatch": ", ".join(
            f"{t}{'...' if parsed_args[i]['is_variadic'] else ''}"
            for i, t in enumerate(concept_types_list_no_dispatch)
        ),
    }

    has_generics = bool(
        generic_base_types["non_variadic"] or generic_base_types["variadic"]
    )
    if has_generics:
        sorted_non_variadic = sorted(list(generic_base_types["non_variadic"]))
        sorted_variadic = [
            f"typename... {t}" for t in sorted(list(generic_base_types["variadic"]))
        ]
        all_generics = [f"typename {t}" for t in sorted_non_variadic] + sorted_variadic
        template_context["arg_types"] = ", ".join(all_generics)
        cpo_template = template_env.get_template("generic_cpo.hpp.jinja2")
    else:
        cpo_template = template_env.get_template("concrete_cpo.hpp.jinja2")

    cpo_definition = cpo_template.render(template_context)

    if generate_doxygen or llm_metadata:
        doxygen_template = template_env.get_template("doxygen.jinja2")
        template_context["cpo_definition"] = cpo_definition
        template_context["param_docs"] = "\n".join(
            [
                f" * @param {arg['name']} [TODO: Description for {arg['name']}]"
                for arg in parsed_args
            ]
        )
        tag_invoke_sig = f"constexpr auto tag_invoke({cpo_name}_ftor, {template_context['arg_pairs']})"
        if has_generics:
            tag_invoke_sig = (
                f"template<{template_context['arg_types']}>\n{tag_invoke_sig}"
            )
        template_context["tag_invoke_signature"] = tag_invoke_sig
        return doxygen_template.render(template_context), {
            "cpo_name": cpo_name,
            "has_generics": has_generics,
            "arg_pairs": template_context["arg_pairs"],
            "arg_types": template_context.get("arg_types", ""),
            "llm_metadata": llm_metadata,
        }
    else:
        return cpo_definition, {
            "cpo_name": cpo_name,
            "has_generics": has_generics,
            "arg_pairs": template_context["arg_pairs"],
            "arg_types": template_context.get("arg_types", ""),
            "llm_metadata": llm_metadata,
        }
