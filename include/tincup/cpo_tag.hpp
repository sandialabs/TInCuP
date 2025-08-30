/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include "cpo_descriptor.hpp"

// Macro to declare CPO metadata - compile-time string literal for powerful metaprogramming
#define TINCUP_CPO_TAG(name_str) \
  static constexpr auto name = ::tincup::StringLiteral{name_str}; \
  static constexpr auto qualified_name() noexcept { \
    return ::tincup::StringLiteral{"tincup::" name_str}; \
  } \
  static constexpr auto descriptor() noexcept { \
    return ::tincup::cpo_descriptor<name>(); \
  }

