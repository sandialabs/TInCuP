/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include <type_traits>
#include <utility>

namespace tincup {

struct BoolDispatch {

  constexpr explicit BoolDispatch( bool value_in ) noexcept : value{value_in} {}

  constexpr operator bool () const noexcept { return value; }

  constexpr BoolDispatch operator ! () const noexcept {
    return BoolDispatch(!value);
  }

  constexpr BoolDispatch operator && ( BoolDispatch other ) const noexcept {
    return BoolDispatch(value && other.value);
  }

  constexpr BoolDispatch operator || ( BoolDispatch other ) const noexcept {
    return BoolDispatch(value || other.value);
  }

  constexpr BoolDispatch operator ^ ( BoolDispatch other ) const noexcept {
    return BoolDispatch(static_cast<bool>(value ^ other.value));
  }

  template<class F>
  decltype(auto) receive( F&& f ) const {
    static_assert(std::is_invocable_v<decltype(f),std::bool_constant<true>>,
                  "Error: Function must be callable with an argument of type std::bool_constant<true>");
    static_assert(std::is_invocable_v<decltype(f),std::bool_constant<false>>,
                  "Error: Function must be callable with an argument of type std::bool_constant<false>");
    if (value) {
      return std::forward<F>(f)(std::bool_constant<true>{});
    } else {
      return std::forward<F>(f)(std::bool_constant<false>{});
    }
  }

  const bool value;
}; // struct BoolDispatch

} // namespace tincup
