# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

LLM_OPERATION_PATTERNS = {
    "mutating_binary": {
        "description": "Modifies first object using second object",
        "args": ["$T&: target", "$const U&: source"],
        "return_constraint": "returns_void_c",
        "semantic_constraints": [
            "std::is_move_constructible_v<T>",  # Target must be movable for modification
        ],
        "requires_mutable": ["target"],  # Mark which args must be mutable
        "example": "// Implement your binary modification logic here",
    },
    "scalar_mutating": {
        "description": "Modifies object using a scalar value",
        "args": ["$T&: target", "$S: scalar"],
        "return_constraint": "returns_void_c",
        "example": "// Implement your scalar modification logic here",
    },
    "unary_mutating": {
        "description": "Modifies object using a unary function",
        "args": ["$T&: target", "$F: func"],
        "return_constraint": "returns_void_c",
        "example": "// Apply func to modify target",
        "implementation_hint": "Apply the function to modify the target object",
    },
    "binary_query": {
        "description": "Computes value from two objects",
        "args": ["$const T&: lhs", "$const U&: rhs"],
        "return_constraint": "returns_value_c",
        "example": "// Implement your binary computation logic here",
    },
    "unary_query": {
        "description": "Computes value from one object",
        "args": ["$const T&: obj"],
        "return_constraint": "returns_value_c",
        "semantic_constraints": [
            "std::is_copy_constructible_v<T>",  # Query operations should preserve object
            "!std::is_void_v<tincup::invocable_t<{cpo_name}_ftor, const T&>>",  # Must return something
        ],
        "requires_const_safe": True,  # Operation shouldn't modify state
        "example": "// Implement your query logic here",
    },
    "generator": {
        "description": "Creates new object from existing object", 
        "args": ["$const T&: source"],
        "return_constraint": "returns_new_object_c",
        "semantic_constraints": [
            "std::is_copy_constructible_v<T>",  # Source must be copyable
            "!std::is_void_v<tincup::invocable_t<{cpo_name}_ftor, const T&>>",  # Must return new object
        ],
        "requires_const_safe": True,  # Should not modify source
        "example": "// Create and return a new object based on source",
    },
    "binary_transform": {
        "description": "Applies a binary function to transform two objects",
        "args": ["$T&: target", "$const U&: source", "$F: func"],
        "return_constraint": "returns_void_c",
        "example": "// Apply func to transform target using source",
        "implementation_hint": "Use the function to transform target based on source",
    },
}


def detect_input_mode(input_data):
    """Detect whether input is from Vim or LLM interface."""
    if "operation_type" in input_data:
        return "LLM_MODE"
    elif "args" in input_data and isinstance(input_data["args"], list):
        return "VIM_MODE"
    else:
        raise ValueError(
            "Invalid input format. Use 'operation_type' for LLM mode or 'args' for Vim mode."
        )


def process_llm_input(input_data):
    """Convert LLM semantic input to Vim-compatible format."""
    cpo_name = input_data["cpo_name"]
    operation_type = input_data["operation_type"]

    if operation_type not in LLM_OPERATION_PATTERNS:
        available = ", ".join(LLM_OPERATION_PATTERNS.keys())
        raise ValueError(
            f"Unknown operation_type '{operation_type}'. Available: {available}"
        )

    pattern = LLM_OPERATION_PATTERNS[operation_type]

    vim_data = {
        "cpo_name": cpo_name,
        "args": pattern["args"],
        "doxygen": input_data.get("doxygen", False),
        "_llm_metadata": {
            "operation_type": operation_type,
            "description": pattern["description"],
            "example_impl": pattern["example"],
            "return_constraint": pattern["return_constraint"],
            "implementation_hint": pattern.get("implementation_hint", ""),
            "semantic_constraints": pattern.get("semantic_constraints", []),
            "requires_const_safe": pattern.get("requires_const_safe", False),
            "requires_mutable": pattern.get("requires_mutable", []),
        },
    }
    return vim_data


def show_llm_help():
    """Display LLM-friendly help information."""
    print("LLM-Friendly CPO Generator: Operation Patterns")
    print("===============================================\n")
    for op_type, info in LLM_OPERATION_PATTERNS.items():
        print(f"  {op_type}: {info['description']}")
        print(f"    Args: {info['args']}\n")
    print("\nExample Usage:")
    print(
        '  cpo-generator \'{"cpo_name": "process", "operation_type": "mutating_binary"}\' --doxygen'
    )
