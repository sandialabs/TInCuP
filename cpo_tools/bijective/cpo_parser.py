#!/usr/bin/env python3
# TInCuP - Bijective Transformation: CPO Parser
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.

"""
Parse TInCuP-generated CPO code using LibClang.

This parser extracts CPO patterns from C++ code without relying on
metadata comments - treating generated code as a black box.

Patterns detected:
1. CPO struct: inline constexpr struct NAME_ftor final { ... } NAME;
2. tag_invoke overload: RETURN_TYPE tag_invoke(NAME_ftor, PARAMS...) { ... }
"""

import re
from dataclasses import dataclass, field
from typing import List, Optional, Dict
from pathlib import Path

try:
    from clang.cindex import Index, CursorKind, TypeKind, Config, ExceptionSpecificationKind
except ImportError:
    raise ImportError(
        "LibClang is required for CPO parsing. Install with: pip install libclang"
    )


@dataclass
class CPOFunction:
    """Represents a parsed CPO with its tag_invoke implementation."""

    function_name: str  # e.g., "serialize"
    ftor_name: str  # e.g., "serialize_ftor"
    return_type: str
    parameters: List[tuple]  # [(type, name), ...]
    template_params: List[str] = field(default_factory=list)
    is_noexcept: bool = False
    namespace: Optional[str] = None

    def to_custom_declaration(self) -> str:
        """Generate a `custom` function declaration."""
        parts = []

        # Template header
        if self.template_params:
            parts.append(f"template<{', '.join(self.template_params)}>")

        # Function signature
        params_str = ", ".join(f"{ptype} {pname}" for ptype, pname in self.parameters)
        noexcept_str = " noexcept" if self.is_noexcept else ""

        func_decl = f"custom {self.return_type} {self.function_name}({params_str}){noexcept_str};"
        parts.append(func_decl)

        return "\n".join(parts)


class CPOParser:
    """
    Parse TInCuP-generated CPO code using LibClang.

    This parser treats TInCuP-generated code as a black box and extracts
    the CPO patterns by analyzing the actual C++ AST structure.
    """

    def __init__(self):
        self.index = Index.create()
        self.cpos: Dict[str, CPOFunction] = {}

    def parse_file(self, filepath: Path, include_paths: List[str] = None) -> List[CPOFunction]:
        """
        Parse a C++ file containing TInCuP-generated CPOs.

        Args:
            filepath: Path to C++ header file
            include_paths: Additional include directories

        Returns:
            List of CPOFunction objects
        """
        # Parse with LibClang
        args = ['-std=c++20']
        if include_paths:
            for inc in include_paths:
                args.append(f'-I{inc}')

        tu = self.index.parse(str(filepath), args=args)

        if not tu:
            raise RuntimeError(f"Failed to parse {filepath}")

        # Check for parse errors
        for diag in tu.diagnostics:
            if diag.severity >= 3:  # Error or Fatal
                print(f"Parse error: {diag.spelling}")

        # Find CPOs and tag_invoke functions
        self.cpos = {}
        self._find_cpos(tu.cursor)
        self._find_tag_invokes(tu.cursor)

        return list(self.cpos.values())

    def _find_cpos(self, cursor):
        """Recursively find CPO struct declarations."""
        # Look for variable declarations like:
        # inline constexpr struct NAME_ftor final { ... } NAME;
        if cursor.kind == CursorKind.VAR_DECL:
            var_name = cursor.spelling

            # Check if this is a CPO pattern (name doesn't end with _ftor)
            if var_name and not var_name.endswith('_ftor'):
                # Get the type - should be a struct ending with _ftor
                type_spelling = cursor.type.spelling

                # Match pattern: "const struct NAME_ftor" or similar
                ftor_match = re.search(r'\b(\w+_ftor)\b', type_spelling)
                if ftor_match:
                    ftor_name = ftor_match.group(1)

                    # Create CPOFunction entry (we'll fill in details from tag_invoke)
                    self.cpos[ftor_name] = CPOFunction(
                        function_name=var_name,
                        ftor_name=ftor_name,
                        return_type="void",  # Default, will be updated
                        parameters=[],
                        namespace=self._get_namespace(cursor)
                    )

        # Recurse
        for child in cursor.get_children():
            self._find_cpos(child)

    def _find_tag_invokes(self, cursor):
        """Recursively find tag_invoke function declarations."""
        if cursor.kind == CursorKind.FUNCTION_DECL or cursor.kind == CursorKind.FUNCTION_TEMPLATE:
            # Check if function name is tag_invoke
            if cursor.spelling == 'tag_invoke':
                self._process_tag_invoke(cursor)

        # Recurse
        for child in cursor.get_children():
            self._find_tag_invokes(child)

    def _process_tag_invoke(self, cursor):
        """Extract information from a tag_invoke function declaration."""
        # Get function parameters
        # For FUNCTION_TEMPLATE, get_arguments() returns empty, so we need to extract from children
        if cursor.kind == CursorKind.FUNCTION_TEMPLATE:
            params = [child for child in cursor.get_children() if child.kind == CursorKind.PARM_DECL]
        else:
            params = list(cursor.get_arguments())

        if not params:
            return  # Invalid tag_invoke

        # First parameter should be the ftor
        first_param_type = params[0].type.spelling

        # Extract ftor name (remove const, &, etc.)
        ftor_match = re.search(r'\b(\w+_ftor)\b', first_param_type)
        if not ftor_match:
            return  # Not a CPO tag_invoke

        ftor_name = ftor_match.group(1)

        if ftor_name not in self.cpos:
            # Found tag_invoke for unknown CPO - create entry
            func_name = ftor_name.replace('_ftor', '')
            self.cpos[ftor_name] = CPOFunction(
                function_name=func_name,
                ftor_name=ftor_name,
                return_type="void",
                parameters=[],
                namespace=self._get_namespace(cursor)
            )

        cpo = self.cpos[ftor_name]

        # Extract return type
        return_type = cursor.result_type.spelling
        # Clean up return type (remove elaborations)
        return_type = re.sub(r'\b(const|struct|class)\s+', '', return_type).strip()
        cpo.return_type = return_type

        # Extract parameters (skip first parameter which is the ftor)
        cpo.parameters = []
        for param in params[1:]:
            param_type = param.type.spelling
            param_name = param.spelling or f"arg{len(cpo.parameters)}"

            # Clean up parameter type
            param_type = self._clean_type(param_type)

            cpo.parameters.append((param_type, param_name))

        # Check for noexcept
        cpo.is_noexcept = cursor.exception_specification_kind in (
            ExceptionSpecificationKind.BASIC_NOEXCEPT,
            ExceptionSpecificationKind.COMPUTED_NOEXCEPT
        )

        # Extract template parameters
        if cursor.kind == CursorKind.FUNCTION_TEMPLATE:
            cpo.template_params = self._extract_template_params(cursor)

    def _extract_template_params(self, cursor) -> List[str]:
        """Extract template parameters from a template function."""
        template_params = []

        for child in cursor.get_children():
            if child.kind == CursorKind.TEMPLATE_TYPE_PARAMETER:
                # typename T, class U, etc.
                param_name = child.spelling
                template_params.append(f"typename {param_name}")
            elif child.kind == CursorKind.TEMPLATE_NON_TYPE_PARAMETER:
                # int N, bool B, etc.
                param_type = child.type.spelling
                param_name = child.spelling
                template_params.append(f"{param_type} {param_name}")
            elif child.kind == CursorKind.TEMPLATE_TEMPLATE_PARAMETER:
                # template<typename> class Container
                param_name = child.spelling
                template_params.append(f"template<typename> class {param_name}")

        return template_params

    def _get_namespace(self, cursor) -> Optional[str]:
        """Get the namespace containing this cursor."""
        namespaces = []
        parent = cursor.semantic_parent

        while parent and parent.kind != CursorKind.TRANSLATION_UNIT:
            if parent.kind == CursorKind.NAMESPACE:
                namespaces.insert(0, parent.spelling)
            parent = parent.semantic_parent

        return '::'.join(namespaces) if namespaces else None

    def _clean_type(self, type_str: str) -> str:
        """Clean up type string (remove elaborations, extra spaces)."""
        # Remove 'const', 'struct', 'class' keywords when redundant
        cleaned = type_str
        # Keep const if it's meaningful (like 'const int &')
        # But remove redundant 'struct' before type names
        cleaned = re.sub(r'\b(struct|class)\s+', '', cleaned)
        cleaned = re.sub(r'\s+', ' ', cleaned).strip()
        return cleaned


def parse_cpo_file(filepath: str, include_paths: List[str] = None) -> List[CPOFunction]:
    """
    Convenience function to parse a CPO file.

    Args:
        filepath: Path to C++ file with TInCuP-generated CPOs
        include_paths: Additional include directories

    Returns:
        List of CPOFunction objects

    Example:
        >>> cpos = parse_cpo_file('generated.hpp')
        >>> for cpo in cpos:
        ...     print(cpo.to_custom_declaration())
    """
    parser = CPOParser()
    return parser.parse_file(Path(filepath), include_paths)


# Example usage
if __name__ == '__main__':
    import sys

    if len(sys.argv) < 2:
        print("Usage: python cpo_parser.py <cpo_file.hpp>")
        sys.exit(1)

    filepath = sys.argv[1]
    cpos = parse_cpo_file(filepath)

    print(f"Found {len(cpos)} CPOs:\n")
    for cpo in cpos:
        print(f"CPO: {cpo.function_name}")
        print(f"  Ftor: {cpo.ftor_name}")
        print(f"  Return: {cpo.return_type}")
        print(f"  Template: {cpo.template_params}")
        print(f"  Params: {cpo.parameters}")
        print(f"  Noexcept: {cpo.is_noexcept}")
        print(f"  Namespace: {cpo.namespace}")
        print(f"\nCustom declaration:")
        print(f"  {cpo.to_custom_declaration()}")
        print()
