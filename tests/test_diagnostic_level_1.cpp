/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#include <iostream>
#include <type_traits>

// Test Level 1: Order and arity diagnostics disabled, pointer/const enabled
#define TINCUP_DIAGNOSTIC_LEVEL 1
#include "../include/tincup/tincup.hpp"

// Verify expected macros at level 1
#ifdef TINCUP_DISABLE_ALL_DIAGNOSTICS
#error "TINCUP_DISABLE_ALL_DIAGNOSTICS should not be defined at level 1"
#endif

#ifdef TINCUP_DISABLE_POINTER_DIAGNOSTICS
#error "TINCUP_DISABLE_POINTER_DIAGNOSTICS should not be defined at level 1"
#endif

#ifdef TINCUP_DISABLE_CONST_DIAGNOSTICS
#error "TINCUP_DISABLE_CONST_DIAGNOSTICS should not be defined at level 1"
#endif

#ifndef TINCUP_DISABLE_ORDER_DIAGNOSTICS
#error "Expected TINCUP_DISABLE_ORDER_DIAGNOSTICS to be defined at level 1"
#endif

#ifndef TINCUP_DISABLE_ARITY_DIAGNOSTICS
#error "Expected TINCUP_DISABLE_ARITY_DIAGNOSTICS to be defined at level 1"
#endif

int main() {
    std::cout << "Diagnostic level 1 test passed\n";
    return 0;
}