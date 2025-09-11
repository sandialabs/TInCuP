/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#include <iostream>
#include <type_traits>

// Test different diagnostic levels by compiling with different macro settings
// This file tests that the diagnostic level macros work correctly

// Test Level 0: All diagnostics disabled
#define TINCUP_DIAGNOSTIC_LEVEL 0
#include "../include/tincup/tincup.hpp"

// Verify that all diagnostic macros are defined at level 0
#ifndef TINCUP_DISABLE_ALL_DIAGNOSTICS
#error "Expected TINCUP_DISABLE_ALL_DIAGNOSTICS to be defined at level 0"
#endif

#ifndef TINCUP_DISABLE_POINTER_DIAGNOSTICS
#error "Expected TINCUP_DISABLE_POINTER_DIAGNOSTICS to be defined at level 0"
#endif

#ifndef TINCUP_DISABLE_CONST_DIAGNOSTICS
#error "Expected TINCUP_DISABLE_CONST_DIAGNOSTICS to be defined at level 0"
#endif

#ifndef TINCUP_DISABLE_ORDER_DIAGNOSTICS
#error "Expected TINCUP_DISABLE_ORDER_DIAGNOSTICS to be defined at level 0"
#endif

#ifndef TINCUP_DISABLE_ARITY_DIAGNOSTICS
#error "Expected TINCUP_DISABLE_ARITY_DIAGNOSTICS to be defined at level 0"
#endif

// Test that we can create a basic CPO - just verify header compiles

int main() {
    std::cout << "Diagnostic level 0 test passed\n";
    return 0;
}