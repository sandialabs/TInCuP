#!/usr/bin/env python3
# TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`
#
# Copyright (c) National Technology & Engineering Solutions of Sandia,
# LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
# Government retains certain rights in this software.
#
# Questions? Contact Greg von Winckel (gvonwin@sandia.gov)

"""
Enhanced CPO Verification and Refactoring Tool

Demonstrates the missing components needed for robust regex-based CPO refactoring.
"""
import re
import os
from typing import List, Dict, Tuple, Optional


class CPOVerifier:
    def __init__(self, filename):
        self.filename = filename
        with open(filename, "r") as f:
            self.content = f.read()
        # Remove comments to avoid false matches
        self.cleaned_content = self.remove_comments(self.content)

    def remove_comments(self, content: str) -> str:
        """Remove C++ comments while preserving string literals."""
        # Remove single-line comments (but not in string literals)
        content = re.sub(r"//.*$", "", content, flags=re.MULTILINE)
        # Remove multi-line comments (but not in string literals)
        content = re.sub(r"/\*.*?\*/", "", content, flags=re.DOTALL)
        return content

    def find_cpo_end(self, start_pos: int) -> int:
        """Find the end of a CPO definition by matching braces."""
        brace_count = 0
        i = start_pos
        found_first_brace = False

        while i < len(self.cleaned_content):
            char = self.cleaned_content[i]
            if char == "{":
                brace_count += 1
                found_first_brace = True
            elif char == "}":
                brace_count -= 1
                if found_first_brace and brace_count == 0:
                    # Look for the semicolon after the closing brace
                    j = i + 1
                    while (
                        j < len(self.cleaned_content)
                        and self.cleaned_content[j].isspace()
                    ):
                        j += 1
                    if j < len(self.cleaned_content) and self.cleaned_content[j] == ";":
                        return j + 1
                    return i + 1
            i += 1
        return len(self.cleaned_content)

    def find_cpo_definitions(self):
        """Find all CPO definitions in the file."""
        # Look for the characteristic pattern of a CPO definition
        # Allow optional namespace qualifiers before cpo_base (e.g., tincup::cpo_base)
        pattern = r"struct\s+(\w+)_ftor\s+final\s*:\s*(?:[\w:]+::)?cpo_base<\1_ftor>"
        matches = re.finditer(pattern, self.cleaned_content)

        cpos = []
        for match in matches:
            cpo_name = match.group(1)
            # Find the boundaries of this CPO definition
            start = match.start()
            # Find the matching closing brace and semicolon
            end = self.find_cpo_end(start)
            cpo_content = self.cleaned_content[start:end]

            cpos.append(
                {"name": cpo_name, "content": cpo_content, "start": start, "end": end}
            )
        return cpos

    def verify_cpo_structure(self, cpo):
        """Verify that a CPO follows the required pattern."""
        errors = []
        name = cpo["name"]
        content = cpo["content"]

        # Check for TINCUP_CPO_TAG
        if f'TINCUP_CPO_TAG("{name}")' not in content:
            errors.append(f"Missing or incorrect TINCUP_CPO_TAG for {name}")

        # Check for the two required operator() overloads
        positive_requires = f"requires tag_invocable_c<{name}_ftor"
        negative_requires = f"requires (!tag_invocable_c<{name}_ftor"

        if positive_requires not in content:
            errors.append(f"Missing positive requires clause for {name}")
        if negative_requires not in content:
            errors.append(f"Missing negative requires clause for {name}")

        # Check for the three concept aliases
        expected_aliases = [
            f"{name}_invocable_c",
            f"{name}_nothrow_invocable_c",
            f"{name}_return_t",
        ]

        for alias in expected_aliases:
            if alias not in content:
                errors.append(f"Missing concept alias: {alias}")

        return errors

    def verify_noexcept_propagation(self, cpo):
        """Verify that noexcept is properly propagated."""
        # Extract the noexcept condition from the first operator()
        noexcept_pattern = r"noexcept\(nothrow_tag_invocable_c<([^>]+)>\)"
        matches = re.findall(noexcept_pattern, cpo["content"])

        if len(matches) != 1:
            return ["Incorrect number of noexcept specifications"]

        # Verify the condition matches the CPO name and arguments
        expected = f"{cpo['name']}_ftor"
        if expected not in matches[0]:
            return [f"Noexcept specification doesn't match CPO name"]

        return []

    def extract_template_parameters(self, cpo) -> List[str]:
        """Extract template parameters from operator() definitions."""
        template_pattern = r"template<([^>]+)>"
        matches = re.findall(template_pattern, cpo["content"])

        if not matches:
            return []

        # Parse the first template declaration
        params = [param.strip() for param in matches[0].split(",")]
        return params

    def extract_function_signature(self, cpo) -> Optional[str]:
        """Extract the function signature from the positive operator() overload."""
        # Look for the positive requires clause operator()
        pattern = r"requires tag_invocable_c<.*?operator\(\)\s*\(([^)]+)\)"
        match = re.search(pattern, cpo["content"], re.DOTALL)

        if match:
            return match.group(1).strip()
        return None

    def verify_forwarding_correctness(self, cpo):
        """Verify that forwarding references are handled correctly."""
        errors = []
        content = cpo["content"]

        # Look for T&& parameters in the signature
        forwarding_refs = re.findall(r"(\w+)&&\s+(\w+)", content)

        for type_name, param_name in forwarding_refs:
            # Check that std::forward<T>(param) is used in tag_invoke call
            forward_pattern = f"std::forward<{type_name}>\\({param_name}\\)"
            if forward_pattern not in content:
                errors.append(
                    f"Missing std::forward<{type_name}>({param_name}) for forwarding reference"
                )

        return errors

    def verify_concept_family_consistency(self, cpo):
        """Verify that all tincup:: concept family uses have identical argument substitution."""
        errors = []
        name = cpo["name"]
        content = cpo["content"]
        
        # Extract all uses of tincup::invocable_c, nothrow_invocable_c, and invocable_t
        invocable_pattern = r"tincup::(invocable_c|nothrow_invocable_c|invocable_t)<([^>]+)>"
        matches = re.findall(invocable_pattern, content)
        
        if not matches:
            return errors  # No concept family uses found
        
        # Normalize whitespace for comparison
        def normalize_args(args_str):
            # Remove all whitespace and normalize for comparison
            return re.sub(r'\s+', '', args_str)
        
        # Group matches by concept type
        invocable_args = []
        nothrow_invocable_args = []
        invocable_t_args = []
        
        for concept_type, args in matches:
            normalized_args = normalize_args(args)
            
            if concept_type == "invocable_c":
                invocable_args.append(normalized_args)
            elif concept_type == "nothrow_invocable_c":
                nothrow_invocable_args.append(normalized_args)
            elif concept_type == "invocable_t":
                invocable_t_args.append(normalized_args)
        
        # All argument lists should be identical (up to whitespace)
        all_args = invocable_args + nothrow_invocable_args + invocable_t_args
        
        if len(set(all_args)) > 1:
            unique_args = list(set(all_args))
            errors.append(
                f"Inconsistent argument substitution in {name} concept family. "
                f"Found {len(unique_args)} different patterns: {unique_args[:2]}..."
            )
        
        # Check that operator() requires/noexcept/return all use the same args
        if all_args:
            canonical_args = all_args[0]
            
            # Pattern to match operator() with all three uses
            operator_pattern = (
                r"requires\s+tincup::invocable_c<([^>]+)>\s*"
                r"constexpr\s+auto\s+operator\(\)\s*\([^)]*\)\s*const\s*"
                r"noexcept\(tincup::nothrow_invocable_c<([^>]+)>\)\s*"
                r"->\s*tincup::invocable_t<([^>]+)>"
            )
            
            operator_matches = re.search(operator_pattern, content, re.DOTALL)
            if operator_matches:
                requires_args = normalize_args(operator_matches.group(1))
                noexcept_args = normalize_args(operator_matches.group(2))  
                return_args = normalize_args(operator_matches.group(3))
                
                if not (requires_args == noexcept_args == return_args == canonical_args):
                    errors.append(
                        f"operator() concept family arguments are inconsistent in {name}. "
                        f"Expected all to be: '{canonical_args}'"
                    )
        
        return errors

    def verify_variadic_flag(self, cpo):
        """Require presence of is_variadic flag and ensure it matches signature ellipses usage."""
        errors = []
        content = cpo["content"]
        name = cpo["name"]

        # Look for is_variadic or (legacy) has_variadic_params
        is_variadic_match = re.search(r"\binline\s+static\s+constexpr\s+bool\s+is_variadic\s*=\s*(true|false)\s*;", content)
        legacy_match = re.search(r"\binline\s+static\s+constexpr\s+bool\s+has_variadic_params\s*=\s*(true|false)\s*;", content)

        if not is_variadic_match and not legacy_match:
            errors.append(f"{name}: missing inline static constexpr bool is_variadic flag (or legacy has_variadic_params)")
            return errors

        flag_value = None
        if is_variadic_match:
            flag_value = (is_variadic_match.group(1) == 'true')
        elif legacy_match:
            flag_value = (legacy_match.group(1) == 'true')

        # Extract operator() parameter lists and detect any '...' occurrences inside
        # This detects template parameter packs in generated signatures
        param_lists = re.findall(r"operator\(\)\s*\(([^)]*)\)", content)
        has_ellipsis = any('...' in params for params in param_lists)

        if flag_value and not has_ellipsis:
            errors.append(f"{name}: is_variadic=true but no parameter pack ('...') found in operator() signatures")
        if not flag_value and has_ellipsis:
            errors.append(f"{name}: is_variadic=false but parameter pack ('...') detected in operator() signatures")

        return errors


class CPOTransformer:
    """Handles automated transformations and refactoring."""

    def __init__(self, content: str):
        self.content = content

    def standardize_spacing(self) -> str:
        """Standardize whitespace in CPO definitions."""
        # Standardize spacing around operators, braces, etc.
        content = re.sub(r"\s*:\s*", " : ", self.content)
        content = re.sub(r"\s*{\s*", " {\n    ", content)
        return content

    def update_cpo_signature(self, cpo_name: str, old_sig: str, new_sig: str) -> str:
        """Update operator() signature across a CPO definition."""
        # This would need more sophisticated logic to handle all occurrences
        pattern = f"(struct\\s+{cpo_name}_ftor.*?operator\\(\\)\\s*\\()[^)]*?\\)"
        replacement = f"\\1{new_sig})"
        return re.sub(pattern, replacement, self.content, flags=re.DOTALL)

    def regenerate_concept_aliases(
        self, cpo_name: str, template_params: str, concept_types: str
    ) -> str:
        """Regenerate the three concept aliases with correct signatures."""
        aliases = f"""
// Readable aliases for {cpo_name} (for external use)
template<{template_params}>
concept {cpo_name}_invocable_c = tag_invocable_c<{cpo_name}_ftor, {concept_types}>;

template<{template_params}>
concept {cpo_name}_nothrow_invocable_c = nothrow_tag_invocable_c<{cpo_name}_ftor, {concept_types}>;

template<{template_params}>
using {cpo_name}_return_t = typename {cpo_name}_ftor::template return_type<{concept_types}>;
"""
        # Replace existing aliases (this would need more robust pattern matching)
        return self.content + aliases


class CPORefactoring:
    """High-level refactoring operations."""

    def __init__(self, project_root: str):
        self.project_root = project_root

    def find_all_cpo_files(self) -> List[str]:
        """Find all files containing CPO definitions."""
        cpo_files = []
        for root, dirs, files in os.walk(self.project_root):
            for file in files:
                if file.endswith((".hpp", ".h", ".cpp", ".cc")):
                    filepath = os.path.join(root, file)
                    with open(filepath, "r") as f:
                        content = f.read()
                        if "cpo_base<" in content:
                            cpo_files.append(filepath)
        return cpo_files

    def migrate_all_cpos(self, migration_rules: Dict[str, str]):
        """Apply migration rules across all CPO files."""
        files = self.find_all_cpo_files()

        for file_path in files:
            verifier = CPOVerifier(file_path)
            cpos = verifier.find_cpo_definitions()

            # Apply transformations
            transformer = CPOTransformer(verifier.content)
            new_content = transformer.content

            for rule_name, rule_pattern in migration_rules.items():
                new_content = re.sub(
                    rule_pattern["from"], rule_pattern["to"], new_content
                )

            # Write back transformed content
            with open(file_path, "w") as f:
                f.write(new_content)


class CPODiagnostic:
    def __init__(self, cpo_name, error_type):
        self.cpo_name = cpo_name
        self.error_type = error_type

    def explain(self):
        explanations = {
            "missing_declare_tag": (
                f"CPO '{self.cpo_name}' is missing TINCUP_CPO_TAG.\n"
                f"WHY THIS MATTERS: The tag declaration provides compile-time\n"
                f"string metadata and standardized error messages.\n"
                f"TO FIX: Regenerate using: cpo-generator '{{'cpo_name': '{self.cpo_name}', ...}}'\n"
            ),
            "inconsistent_structure": (
                f"CPO '{self.cpo_name}' has a different structure than others.\n"
                f"WHY THIS MATTERS: Structural consistency enables generic\n"
                f"tooling and makes the codebase predictable.\n"
                f"TO FIX: Review the CPO and consider regenerating it.\n"
            ),
            "missing_forwarding": (
                f"CPO '{self.cpo_name}' has forwarding reference parameters but doesn't use std::forward.\n"
                f"WHY THIS MATTERS: Perfect forwarding preserves value categories.\n"
                f"TO FIX: Use std::forward<T>(param) for T&& parameters.\n"
            ),
        }
        return explanations.get(self.error_type, "Unknown error")


def main():
    """Example usage of the verification system."""
    if len(os.sys.argv) < 2:
        print("Usage: python cpo_verification.py <file_or_directory>")
        return

    target = os.sys.argv[1]

    if os.path.isfile(target):
        verifier = CPOVerifier(target)
        cpos = verifier.find_cpo_definitions()

        for cpo in cpos:
            errors = verifier.verify_cpo_structure(cpo)
            errors.extend(verifier.verify_noexcept_propagation(cpo))
            errors.extend(verifier.verify_forwarding_correctness(cpo))
            errors.extend(verifier.verify_concept_family_consistency(cpo))
            errors.extend(verifier.verify_variadic_flag(cpo))

            if errors:
                print(f"CPO '{cpo['name']}' has issues:")
                for error in errors:
                    print(f"  - {error}")
            else:
                print(f"CPO '{cpo['name']}' is valid ✓")

    elif os.path.isdir(target):
        refactoring = CPORefactoring(target)
        files = refactoring.find_all_cpo_files()
        print(f"Found {len(files)} files with CPOs")


if __name__ == "__main__":
    main()
