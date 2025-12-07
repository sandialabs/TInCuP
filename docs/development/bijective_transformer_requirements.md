# Bijective Transformer Requirements for TInCuP

## Executive Summary

To support a **bijective transformation** between `custom` function declarations and TInCuP-generated CPO+tag_invoke code, we need to enhance TInCuP with:

1. **Canonical Structure Enforcement**: Strict naming and structure conventions
2. **Metadata Preservation**: Embedded markers for round-trip identity
3. **CPO→Custom Parser**: Inverse transformer to recover `custom` declarations
4. **Validation Framework**: Round-trip identity verification

This document outlines what needs to be added to TInCuP to enable this capability.

## Current TInCuP Capabilities

### ✅ What TInCuP Already Provides

1. **Python Code Generator** (`cpo-generator`):
   - Generates CPO boilerplate from JSON/YAML specs
   - Jinja2 templates for flexible output
   - Registry support for tracking existing CPOs

2. **C++20 Header Library**:
   - `cpo_base` CRTP base class
   - `TINCUP_CPO_TAG` macro for metadata
   - Diagnostic support
   - Type traits and concepts

3. **Verification Tools**:
   - `cpo_verification.py`: Checks CPO structure
   - Registry scanner: Finds existing CPOs in headers

4. **Flexibility**:
   - Support for generic and concrete CPOs
   - Template parameter handling
   - Namespace support
   - Third-party type integration

## Gap Analysis: What's Missing for Bijectivity

### 1. **Strict Canonical Structure** ❌

**Current State**: TInCuP is flexible - allows variations in CPO structure

**Needed for Bijectivity**: Enforce exactly one canonical pattern

**Required Changes**:

```python
# New module: cpo_tools/canonical_structure.py

class CanonicalCPOStructure:
    """
    Enforces strict canonical structure for bijective transformation.

    Rules:
    1. CPO struct name must be: <name>_ftor or <name>_fn
    2. CPO variable name must match function name exactly
    3. Must use TINCUP_CPO_TAG with matching name
    4. CPO must be in detail:: namespace or inline
    5. tag_invoke first param must be CPO type
    """

    FUNCTOR_SUFFIX = "_ftor"  # Preferred
    ALTERNATE_SUFFIX = "_fn"  # Also acceptable

    @staticmethod
    def validate_cpo_name(cpo_name: str, struct_name: str) -> Result:
        """Verify struct name follows canonical pattern."""
        expected = f"{cpo_name}{CanonicalCPOStructure.FUNCTOR_SUFFIX}"
        alternate = f"{cpo_name}{CanonicalCPOStructure.ALTERNATE_SUFFIX}"

        if struct_name not in (expected, alternate):
            return Err(f"Struct name must be '{expected}' or '{alternate}', got '{struct_name}'")
        return Ok()

    @staticmethod
    def validate_namespace_structure(namespace: Optional[str]) -> Result:
        """Verify namespace follows canonical pattern."""
        # Either: no namespace (inline), or detail::
        if namespace and namespace != "detail":
            return Err(f"CPO namespace must be 'detail' or inline, got '{namespace}'")
        return Ok()
```

### 2. **Metadata Preservation Markers** ❌

**Current State**: Generated code has no embedded metadata for reverse transformation

**Needed**: Machine-readable comments or attributes that preserve:
- Original signature
- Template parameters
- Noexcept specifications
- Namespace context
- Generation metadata

**Required Changes**:

```python
# Add to templates/generic_cpo.hpp.jinja2

{% if enable_bijective_metadata %}
// TINCUP_GENERATED_FROM_CUSTOM: v{{ tincup_version }}
// TINCUP_ORIGINAL_SIGNATURE: {{ original_custom_signature }}
// TINCUP_TEMPLATE_PARAMS: {{ template_param_list }}
// TINCUP_NOEXCEPT: {{ is_noexcept }}
{% endif %}
inline constexpr struct {{ cpo_name }}_ftor final : tincup::cpo_base<{{ cpo_name }}_ftor> {
  TINCUP_CPO_TAG("{{ cpo_name }}")
  // ... rest of CPO
} {{ cpo_name }};

{% if enable_bijective_metadata %}
// TINCUP_TAG_INVOKE_BEGIN: {{ cpo_name }}
{% endif %}
template<{{ arg_types }}>
void tag_invoke({{ cpo_name }}_ftor, {{ arg_pairs }}) {
  // ... body
}
{% if enable_bijective_metadata %}
// TINCUP_TAG_INVOKE_END: {{ cpo_name }}
{% endif %}
```

**Alternative**: Use C++20 attributes (if we can parse them reliably):

```cpp
[[tincup::generated_from_custom, tincup::version("1.0.0")]]
inline constexpr struct serialize_ftor final : tincup::cpo_base<serialize_ftor> {
  // ...
};
```

### 3. **Enhanced Registry with Full Signatures** ❌

**Current State**: Registry only tracks CPO names and locations

**Needed**: Full signature information for reconstruction

**Required Changes**:

```python
# Enhance cpo_tools/cpo_registry.py

@dataclass
class CPOEntry:
    name: str
    qualified: str
    header: str
    struct: Optional[str]
    line: int

    # NEW FIELDS for bijectivity:
    template_params: List[str]  # ["typename T", "int N"]
    tag_invoke_signatures: List[TagInvokeSignature]  # All overloads
    namespace: Optional[str]  # "detail" or None
    cpo_variable_name: str  # Usually same as name
    metadata_markers: Dict[str, str]  # Parsed metadata

@dataclass
class TagInvokeSignature:
    """Represents one tag_invoke overload."""
    template_params: List[str]
    return_type: str
    parameters: List[Parameter]
    is_noexcept: bool
    body_hash: Optional[str]  # For change detection

@dataclass
class Parameter:
    type_str: str  # Full type including cv-qualifiers and references
    name: str
    is_pack: bool
```

### 4. **CPO → Custom Parser** ❌ CRITICAL

**Current State**: No inverse transformation exists

**Needed**: Parse compliant CPO code and extract `custom` declarations

**Required Implementation**:

```python
# New module: cpo_tools/cpo_to_custom.py

from dataclasses import dataclass
from typing import List, Optional
import re

class CPOParser:
    """
    Parse TInCuP-compliant CPO code to extract metadata.
    """

    def parse_cpo_declaration(self, code: str) -> CPODeclaration:
        """
        Parse CPO struct declaration.

        Expected pattern:
        inline constexpr struct <name>_ftor final : tincup::cpo_base<...> {
          TINCUP_CPO_TAG("<name>")
          ...
        } <variable_name>;
        """
        pass

    def parse_tag_invoke_overloads(self, code: str, cpo_name: str) -> List[TagInvokeSignature]:
        """
        Find all tag_invoke overloads for a given CPO.

        Expected pattern:
        template<typename T, ...>
        ReturnType tag_invoke(<cpo_name>_ftor, Args...) [noexcept] {
          ...
        }
        """
        pass

    def extract_metadata_markers(self, code: str) -> Dict[str, str]:
        """
        Extract TINCUP_GENERATED_FROM_CUSTOM markers.
        """
        metadata = {}

        # Pattern: // TINCUP_KEY: value
        pattern = r'//\s*TINCUP_(\w+):\s*(.+)$'

        for match in re.finditer(pattern, code, re.MULTILINE):
            key = match.group(1)
            value = match.group(2).strip()
            metadata[key] = value

        return metadata

class CustomReconstructor:
    """
    Reconstruct `custom` function declarations from CPO code.
    """

    def reconstruct(self, cpo_decl: CPODeclaration,
                   tag_invokes: List[TagInvokeSignature]) -> List[str]:
        """
        Generate `custom` function declarations.

        Returns:
            List of custom function declaration strings
        """
        custom_decls = []

        for ti in tag_invokes:
            # Skip first parameter (CPO tag)
            params = ti.parameters[1:]

            # Build template header if needed
            template_header = ""
            if ti.template_params:
                template_header = f"template<{', '.join(ti.template_params)}>\n"

            # Build parameter list
            param_list = ", ".join(f"{p.type_str} {p.name}" for p in params)

            # Build noexcept clause
            noexcept_clause = " noexcept" if ti.is_noexcept else ""

            # Assemble custom declaration
            custom_decl = (
                f"{template_header}"
                f"custom {ti.return_type} {cpo_decl.name}({param_list}){noexcept_clause};"
            )

            custom_decls.append(custom_decl)

        return custom_decls
```

### 5. **Round-Trip Validation Framework** ❌

**Current State**: No automated verification of transformation identity

**Needed**: Test framework that validates bijection property

**Required Implementation**:

```python
# New module: cpo_tools/bijective_validator.py

class BijectiveValidator:
    """
    Validate round-trip transformation identity.

    Tests:
    1. custom → CPO → custom yields identical result
    2. CPO → custom → CPO yields identical result (for canonical CPOs)
    """

    def validate_custom_roundtrip(self,
                                  original_custom: str,
                                  custom_to_cpo_fn: Callable,
                                  cpo_to_custom_fn: Callable) -> ValidationResult:
        """
        Test: custom → CPO → custom
        """
        # Generate CPO from custom
        cpo_code = custom_to_cpo_fn(original_custom)

        # Reconstruct custom from CPO
        reconstructed_custom = cpo_to_custom_fn(cpo_code)

        # Compare (after normalization)
        if self.normalize_custom(original_custom) == self.normalize_custom(reconstructed_custom):
            return ValidationResult.success()
        else:
            return ValidationResult.failure(
                original=original_custom,
                reconstructed=reconstructed_custom,
                diff=self.compute_diff(original_custom, reconstructed_custom)
            )

    def validate_cpo_roundtrip(self,
                               original_cpo: str,
                               cpo_to_custom_fn: Callable,
                               custom_to_cpo_fn: Callable) -> ValidationResult:
        """
        Test: CPO → custom → CPO

        Note: Only valid for canonical CPOs
        """
        # Check if CPO is canonical
        if not self.is_canonical(original_cpo):
            return ValidationResult.skip("Non-canonical CPO")

        # Extract custom from CPO
        custom_code = cpo_to_custom_fn(original_cpo)

        # Regenerate CPO from custom
        regenerated_cpo = custom_to_cpo_fn(custom_code)

        # Compare (semantic equivalence)
        if self.semantically_equivalent(original_cpo, regenerated_cpo):
            return ValidationResult.success()
        else:
            return ValidationResult.failure(
                original=original_cpo,
                regenerated=regenerated_cpo,
                diff=self.compute_semantic_diff(original_cpo, regenerated_cpo)
            )

    @staticmethod
    def normalize_custom(code: str) -> str:
        """Normalize custom code for comparison."""
        # Remove whitespace variations
        # Normalize type aliases
        # Sort template parameters (if order doesn't matter)
        pass

    @staticmethod
    def semantically_equivalent(cpo1: str, cpo2: str) -> bool:
        """Check if two CPOs are semantically equivalent."""
        # Compare ASTs or canonical representations
        pass
```

### 6. **Command-Line Interface Extensions** ❌

**Current State**: `cpo-generator` generates CPO from specs

**Needed**: Bidirectional transformation CLI

**Required Implementation**:

```bash
# New commands:

# Forward: custom → CPO
cpo-generator --mode=from-custom \
  --input=my_functions.hpp \
  --output=my_cpos.hpp \
  --enable-bijective-metadata \
  --validate-roundtrip

# Reverse: CPO → custom
cpo-generator --mode=to-custom \
  --input=my_cpos.hpp \
  --output=my_functions_recovered.hpp \
  --verify-canonical

# Validation
cpo-generator --validate-bijection \
  --custom-input=original.hpp \
  --cpo-input=generated.hpp \
  --report=validation_report.json
```

**Implementation**:

```python
# Add to cpo_tools/src/args.py

def parse_args():
    parser = argparse.ArgumentParser(...)

    # ... existing args ...

    # NEW: Bijective transformation args
    parser.add_argument('--mode',
                       choices=['from-json', 'from-custom', 'to-custom'],
                       default='from-json',
                       help='Transformation mode')

    parser.add_argument('--enable-bijective-metadata',
                       action='store_true',
                       help='Embed metadata for round-trip transformation')

    parser.add_argument('--validate-roundtrip',
                       action='store_true',
                       help='Validate round-trip identity after generation')

    parser.add_argument('--verify-canonical',
                       action='store_true',
                       help='Verify input CPO follows canonical structure')

    parser.add_argument('--validate-bijection',
                       action='store_true',
                       help='Run full bijection validation')

    parser.add_argument('--custom-input',
                       type=Path,
                       help='Custom function declarations (for validation)')

    parser.add_argument('--cpo-input',
                       type=Path,
                       help='CPO code (for validation or reverse transform)')
```

## Implementation Roadmap

### Phase 1: Foundation (Week 1-2)

1. **Canonical Structure Definition**
   - Document exact canonical pattern
   - Create validation functions
   - Add to existing `cpo_verification.py`

2. **Metadata Markers**
   - Design marker format
   - Add to Jinja2 templates
   - Make optional with flag

3. **Enhanced Registry**
   - Extend `CPOEntry` dataclass
   - Update parser in `cpo_registry.py`
   - Add signature extraction logic

### Phase 2: Reverse Transformation (Week 3-4)

4. **CPO Parser**
   - Implement `CPOParser` class
   - Handle template parameters
   - Extract tag_invoke signatures

5. **Custom Reconstructor**
   - Implement `CustomReconstructor`
   - Handle all C++20 features
   - Preserve formatting where possible

6. **CLI Integration**
   - Add `--mode=to-custom` support
   - Implement reverse transformation flow
   - Add progress reporting

### Phase 3: Validation (Week 5-6)

7. **Round-Trip Validator**
   - Implement `BijectiveValidator`
   - Add comprehensive test cases
   - Create validation report generator

8. **Integration Testing**
   - Test suite for all C++20 features
   - Edge case handling
   - Performance benchmarks

### Phase 4: Integration with Clang Transformer (Week 7-8)

9. **Clang Tool Integration**
   - Create clang-based `custom` extractor
   - Feed extracted data to TInCuP generator
   - Handle complex template scenarios

10. **Documentation & Examples**
    - User guide for bijective transformation
    - Migration workflows
    - Best practices

## Acceptance Criteria

A successful implementation must:

1. ✅ **Generate canonical CPOs** from `custom` declarations
2. ✅ **Reconstruct identical `custom` code** from canonical CPOs
3. ✅ **Validate round-trip identity** automatically
4. ✅ **Handle all C++20 features**: templates, concepts, noexcept, etc.
5. ✅ **Provide clear error messages** for non-canonical CPOs
6. ✅ **Support batch transformation** of entire codebases
7. ✅ **Generate validation reports** for standards committee

## Example Workflow

```bash
# 1. Write code with custom (using modified clang)
cat > my_lib.hpp << 'EOF'
template<typename T>
custom void serialize(const T& obj, std::ostream& os);

custom int compute(double x) noexcept;
EOF

# 2. Generate CPO version with metadata
cpo-generator --mode=from-custom \
  --input=my_lib.hpp \
  --output=my_lib_cpo.hpp \
  --enable-bijective-metadata \
  --validate-roundtrip

# 3. Validate canonical structure
cpo-generator --verify-canonical my_lib_cpo.hpp
# ✓ All CPOs follow canonical structure

# 4. Reverse transformation
cpo-generator --mode=to-custom \
  --input=my_lib_cpo.hpp \
  --output=my_lib_recovered.hpp

# 5. Verify identity
diff my_lib.hpp my_lib_recovered.hpp
# (should be identical after normalization)

# 6. Full bijection validation
cpo-generator --validate-bijection \
  --custom-input=my_lib.hpp \
  --cpo-input=my_lib_cpo.hpp \
  --report=validation.json

cat validation.json
{
  "status": "success",
  "total_functions": 2,
  "roundtrip_passed": 2,
  "roundtrip_failed": 0,
  "canonical_violations": 0
}
```

## Questions for Discussion

1. **Metadata Format**: Comments vs. C++20 attributes?
   - **Recommendation**: Start with comments (easier to parse), migrate to attributes later

2. **Strictness Level**: How strictly enforce canonical structure?
   - **Recommendation**: Very strict for bijection, provide flag to relax for compatibility

3. **Namespace Handling**: Require `detail::` namespace or allow flexibility?
   - **Recommendation**: Canonical uses `detail::`, validator accepts both patterns

4. **Body Preservation**: Should we preserve tag_invoke body or just signature?
   - **Recommendation**: Signature only for bijection, body stays in implementation

5. **Version Compatibility**: How to handle TInCuP version changes?
   - **Recommendation**: Embed version in metadata, provide migration tools

## Next Steps

1. Review this requirements doc
2. Prioritize features (MVP vs. nice-to-have)
3. Create detailed design doc for Phase 1
4. Set up development branch in TInCuP
5. Begin implementation with canonical structure validation

---

**Author**: Analysis based on TInCuP v1.0.4
**Date**: 2025-12-05
**Status**: Proposed - Awaiting Review
