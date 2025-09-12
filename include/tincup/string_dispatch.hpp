/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include <concepts>
#include <string_view>
#include "cpo_tag.hpp"

namespace tincup {

template<std::size_t N> 
using string_view_array = std::array<std::string_view, N>;


template<auto Value>
struct constant_t {
  static constexpr auto value = Value;
};

template<auto LhsValue, auto RhsValue>
constexpr bool operator == ( constant_t<LhsValue>, 
		             constant_t<RhsValue> ) {
  return LhsValue == RhsValue;
}

template<auto LhsValue, std::integral Rhs>
constexpr bool operator == ( constant_t<LhsValue>, 
		             const Rhs& rhs ) {
  return LhsValue == rhs;
}

template<std::integral Lhs, auto RhsValue>
constexpr bool operator == ( const Lhs& lhs, 
		             constant_t<RhsValue> ) {
  return lhs == RhsValue;
}


template<std::size_t N>
class StringDispatch {
public:
  constexpr explicit StringDispatch( std::string_view runtime_value,
                                     const string_view_array<N>& options ) noexcept
    : value(runtime_value), opts(options) {}

  template<class F>
  constexpr decltype(auto) receive( F&& f ) const {
    return dispatch_index<0>(std::forward<F>(f));
  }

private:

  template<std::size_t I, class F>
  constexpr decltype(auto) dispatch_index( F&& f ) const {
    if constexpr (I < N) {
      if (opts[I] == value) {
        return std::forward<F>(f)(std::integral_constant<std::size_t, I>{});
      } else {
        return dispatch_index<I + 1>(std::forward<F>(f));
      }
    } else {
      // Not found path: pass N as the index
      return std::forward<F>(f)(std::integral_constant<std::size_t, N>{});
    }
  }
  const std::string_view value;
  const string_view_array<N>& opts;

}; // class StringDispatch

} // namespace tincup
