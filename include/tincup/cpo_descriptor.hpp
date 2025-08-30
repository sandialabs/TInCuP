/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include <string_view>
#include "string_literal.hpp"

namespace tincup {

// CPO Registry Infrastructure - enables compile-time CPO discovery and metadata
template<StringLiteral Name>
struct cpo_descriptor {
  static constexpr auto name = Name;
  static constexpr const char* c_str() noexcept {
    return Name.value.data();
  }
  static constexpr std::string_view view() noexcept {
    return std::string_view{Name.value.data(), Name.value.size() - 1}; // -1 to exclude null terminator
  }
};

} // namespace tincup
