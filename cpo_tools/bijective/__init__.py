"""
TInCuP Bijective Transformation

This package provides bidirectional transformation between:
- C++ `custom` function declarations (C++29 proposal)
- CPO + tag_invoke boilerplate (C++20 compatible via TInCuP)

Forward transformation (custom → CPO):
    - Uses regex-based parser (no dependencies)
    - Always available

Reverse transformation (CPO → custom):
    - Uses LibClang-based parser
    - Requires: pip install libclang or TInCuP[reverse]
"""

from .custom_parser import CustomFunctionParser, CustomFunction, parse_custom_file
from .transformer import CustomToCPOTransformer, transform_custom_to_cpo

# Lazy imports for reverse transformation (optional libclang dependency)
try:
    from .cpo_parser import CPOParser, CPOFunction, parse_cpo_file
    from .reverse_transformer import CPOToCustomTransformer, transform_cpo_to_custom
    _REVERSE_AVAILABLE = True
except ImportError:
    _REVERSE_AVAILABLE = False

__all__ = [
    # Forward transformation (always available)
    'CustomFunctionParser',
    'CustomFunction',
    'parse_custom_file',
    'CustomToCPOTransformer',
    'transform_custom_to_cpo',
]

# Add reverse transformation exports if available
if _REVERSE_AVAILABLE:
    __all__.extend([
        'CPOParser',
        'CPOFunction',
        'parse_cpo_file',
        'CPOToCustomTransformer',
        'transform_cpo_to_custom',
    ])
