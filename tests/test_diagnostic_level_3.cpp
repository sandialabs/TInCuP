/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#include <iostream>
#include <type_traits>

// Test Level 3: All diagnostics enabled (default)
#define TINCUP_DIAGNOSTIC_LEVEL 3
#include "../include/tincup/tincup.hpp"

// Verify no diagnostic disable macros are defined at level 3
#ifdef TINCUP_DISABLE_ALL_DIAGNOSTICS
#error "TINCUP_DISABLE_ALL_DIAGNOSTICS should not be defined at level 3"
#endif

#ifdef TINCUP_DISABLE_POINTER_DIAGNOSTICS
#error "TINCUP_DISABLE_POINTER_DIAGNOSTICS should not be defined at level 3"
#endif

#ifdef TINCUP_DISABLE_CONST_DIAGNOSTICS
#error "TINCUP_DISABLE_CONST_DIAGNOSTICS should not be defined at level 3"
#endif

#ifdef TINCUP_DISABLE_ORDER_DIAGNOSTICS
#error "TINCUP_DISABLE_ORDER_DIAGNOSTICS should not be defined at level 3"
#endif

#ifdef TINCUP_DISABLE_ARITY_DIAGNOSTICS
#error "TINCUP_DISABLE_ARITY_DIAGNOSTICS should not be defined at level 3"
#endif

int main() {
    std::cout << "Diagnostic level 3 test passed\n";
    return 0;
}