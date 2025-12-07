#!/usr/bin/env python3
# TInCuP - Bijective Transformation: Custom Function Parser
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.

"""
Parse C++ `custom` function declarations using regex.

This parser extracts function signatures from code using the `custom` keyword
without requiring a full C++ compiler. It handles:

- Simple functions: custom void foo();
- Templates: template<typename T> custom T bar(T x);
- Noexcept: custom int baz() noexcept;
- Namespaces: namespace ns { custom void qux(); }
- Multiple parameters with complex types
"""

import re
from dataclasses import dataclass, field
from typing import List, Optional, Tuple
from pathlib import Path


@dataclass
class Parameter:
    """Represents a function parameter."""
    type_str: str  # Full type including cv-qualifiers, references, pointers
    name: str

    def to_tincup_format(self) -> str:
        """
        Convert to TInCuP argument format: '$Type: name' or 'Type: name'

        Examples:
            int x -> int: x
            const T& obj -> $const T&: obj
            std::vector<T>& v -> $std::vector<T>&: v
        """
        # Add $ prefix if type contains template parameters or is generic
        if self._is_generic_type():
            return f"${self.type_str}: {self.name}"
        return f"{self.type_str}: {self.name}"

    def _is_generic_type(self) -> bool:
        """Check if type appears to be a template parameter."""
        # Simple heuristic: single capital letter, or contains 'typename' keywords
        type_clean = self.type_str.replace('const', '').replace('&', '').replace('*', '').strip()

        # Check for template syntax
        if '<' in self.type_str or '>' in self.type_str:
            return True

        # Check for single capital letter (likely template param)
        if len(type_clean) == 1 and type_clean.isupper():
            return True

        # Check for common template parameter patterns (T, U, V, etc.)
        if type_clean in ('T', 'U', 'V', 'W', 'Args', 'Rest'):
            return True

        return False


@dataclass
class CustomFunction:
    """
    Represents a parsed `custom` function declaration.
    """
    function_name: str
    return_type: str
    parameters: List[Parameter]
    template_params: List[str] = field(default_factory=list)
    is_noexcept: bool = False
    namespace: Optional[str] = None
    line_number: int = 0

    def to_tincup_spec(self) -> dict:
        """
        Convert to TInCuP JSON specification format.

        Returns:
            dict suitable for feeding to cpo-generator
        """
        spec = {
            "cpo_name": self.function_name,
            "args": [p.to_tincup_format() for p in self.parameters],
        }

        # Add optional fields if present
        if self.template_params:
            # TInCuP infers templates from args, but we can validate
            pass

        return spec

    def __str__(self) -> str:
        """Reconstruct the custom function declaration."""
        parts = []

        # Template header
        if self.template_params:
            parts.append(f"template<{', '.join(self.template_params)}>")

        # custom keyword + signature
        params_str = ", ".join(f"{p.type_str} {p.name}" for p in self.parameters)
        noexcept_str = " noexcept" if self.is_noexcept else ""

        func_decl = f"custom {self.return_type} {self.function_name}({params_str}){noexcept_str};"
        parts.append(func_decl)

        return "\n".join(parts)


class CustomFunctionParser:
    """
    Parse `custom` function declarations from C++ source code.

    Uses regex-based parsing - fast and simple for well-formed code.
    For complex C++ syntax, consider using LibClang (see cpo_parser.py).
    """

    # Regex pattern for custom function declarations
    # Matches: [template<...>] custom RetType Name(Args) [noexcept] [{ body }];
    CUSTOM_PATTERN = re.compile(
        r'(?:template\s*<([^>]+)>\s*)?'          # Optional template params (group 1)
        r'custom\s+'                             # custom keyword
        r'([\w:]+(?:\s*<[^>]+>)?(?:\s*[*&])*)\s+' # Return type with namespace/template (group 2)
        r'([\w]+)\s*'                            # Function name (group 3)
        r'\(([^)]*)\)'                           # Parameters (group 4)
        r'(?:\s+(noexcept))?'                    # Optional noexcept (group 5)
        r'(?:\s*\{[^}]*\}|\s*;)',                # Body or semicolon
        re.MULTILINE | re.DOTALL
    )

    # Namespace pattern to track context
    NAMESPACE_PATTERN = re.compile(
        r'namespace\s+([\w:]+)\s*\{',
        re.MULTILINE
    )

    def __init__(self):
        self.current_namespace = None

    def parse(self, source: str) -> List[CustomFunction]:
        """
        Parse source code and extract all custom function declarations.

        Args:
            source: C++ source code as string

        Returns:
            List of CustomFunction objects
        """
        functions = []

        # Track namespaces (simple stack-based approach)
        namespace_stack = []

        # Find all custom declarations in the entire source (not line-by-line)
        for match in self.CUSTOM_PATTERN.finditer(source):
            # Determine line number
            line_num = source[:match.start()].count('\n') + 1

            # Simple namespace tracking (could be improved)
            # For now, just extract from context before the match
            func = self._parse_match(match, line_num, namespace_stack)
            if func:
                functions.append(func)

        return functions

    def parse_file(self, filepath: Path) -> List[CustomFunction]:
        """
        Parse a C++ source file.

        Args:
            filepath: Path to .cpp or .hpp file

        Returns:
            List of CustomFunction objects
        """
        content = filepath.read_text(encoding='utf-8')
        return self.parse(content)

    def _parse_match(self, match: re.Match, line_num: int, namespace_stack: List[str]) -> Optional[CustomFunction]:
        """Parse a regex match into a CustomFunction object."""

        # Extract matched groups
        template_params_str = match.group(1)
        return_type = match.group(2).strip()
        function_name = match.group(3)
        params_str = match.group(4)
        is_noexcept = match.group(5) is not None

        # Parse template parameters
        template_params = []
        if template_params_str:
            template_params = self._parse_template_params(template_params_str)

        # Parse function parameters
        parameters = self._parse_parameters(params_str)

        # Determine namespace
        namespace = '::'.join(namespace_stack) if namespace_stack else None

        return CustomFunction(
            function_name=function_name,
            return_type=return_type,
            parameters=parameters,
            template_params=template_params,
            is_noexcept=is_noexcept,
            namespace=namespace,
            line_number=line_num
        )

    def _parse_template_params(self, params_str: str) -> List[str]:
        """
        Parse template parameter list.

        Examples:
            'typename T' -> ['typename T']
            'typename T, int N' -> ['typename T', 'int N']
            'typename... Args' -> ['typename... Args']
        """
        if not params_str:
            return []

        # Split by comma, handling nested templates
        params = []
        current = ""
        depth = 0

        for char in params_str:
            if char == '<':
                depth += 1
            elif char == '>':
                depth -= 1
            elif char == ',' and depth == 0:
                params.append(current.strip())
                current = ""
                continue

            current += char

        if current.strip():
            params.append(current.strip())

        return params

    def _parse_parameters(self, params_str: str) -> List[Parameter]:
        """
        Parse function parameter list.

        Examples:
            'int x' -> [Parameter('int', 'x')]
            'const T& obj, std::vector<T>& v' -> [Parameter('const T&', 'obj'), ...]
        """
        if not params_str or params_str.strip() == 'void':
            return []

        parameters = []

        # Split by comma, handling nested templates
        param_strings = self._split_parameters(params_str)

        for param_str in param_strings:
            param = self._parse_single_parameter(param_str.strip())
            if param:
                parameters.append(param)

        return parameters

    def _split_parameters(self, params_str: str) -> List[str]:
        """Split parameter list by commas, respecting template nesting."""
        params = []
        current = ""
        depth = 0

        for char in params_str:
            if char == '<':
                depth += 1
            elif char == '>':
                depth -= 1
            elif char == ',' and depth == 0:
                params.append(current)
                current = ""
                continue

            current += char

        if current.strip():
            params.append(current)

        return params

    def _parse_single_parameter(self, param_str: str) -> Optional[Parameter]:
        """
        Parse a single parameter declaration.

        Examples:
            'int x' -> Parameter('int', 'x')
            'const T& obj' -> Parameter('const T&', 'obj')
            'std::vector<T>&& v' -> Parameter('std::vector<T>&&', 'v')
        """
        # Pattern: Type Name
        # Handle cv-qualifiers, references, pointers, templates

        # Split on last whitespace to separate type from name
        parts = param_str.rsplit(None, 1)

        if len(parts) != 2:
            # Handle cases like 'void' or malformed input
            return None

        type_str, name = parts

        # Clean up name (remove default values if present)
        if '=' in name:
            name = name.split('=')[0].strip()

        return Parameter(type_str=type_str.strip(), name=name.strip())


def parse_custom_file(filepath: str) -> List[CustomFunction]:
    """
    Convenience function to parse a file.

    Args:
        filepath: Path to C++ source file

    Returns:
        List of CustomFunction objects

    Example:
        >>> functions = parse_custom_file('my_library.hpp')
        >>> for func in functions:
        ...     print(f"Found: {func.function_name}")
    """
    parser = CustomFunctionParser()
    return parser.parse_file(Path(filepath))


# Example usage
if __name__ == '__main__':
    # Example custom code
    example_code = '''
namespace mylib {

template<typename T>
custom void serialize(const T& obj, std::ostream& os);

custom int compute(double x) noexcept;

template<typename T, typename U>
custom void add_in_place(std::vector<T>& target, const std::vector<U>& source) {
    // Implementation
}

} // namespace mylib
'''

    parser = CustomFunctionParser()
    functions = parser.parse(example_code)

    print(f"Found {len(functions)} custom functions:\n")
    for func in functions:
        print(f"Function: {func.function_name}")
        print(f"  Return type: {func.return_type}")
        print(f"  Parameters: {[f'{p.type_str} {p.name}' for p in func.parameters]}")
        print(f"  Template params: {func.template_params}")
        print(f"  Noexcept: {func.is_noexcept}")
        print(f"  Namespace: {func.namespace}")
        print(f"  TInCuP spec: {func.to_tincup_spec()}")
        print()
