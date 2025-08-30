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

template<typename T>
concept has_deref_c = requires ( T&& x ) {
  { *x };
};

template<has_deref_c T>
auto deref_if_needed(T&& x) noexcept(noexcept(*std::forward<T>(x)))
-> decltype(*std::forward<T>(x)) {
  return *std::forward<T>(x);
}
template<typename T>
requires (!has_deref_c<T>)
auto deref_if_needed(T&& x) noexcept(std::is_nothrow_move_constructible_v<T>)
-> T&& {
  return std::forward<T>(x);
}

template<typename T>
using deref_t = decltype(deref_if_needed(std::declval<T>()));

} // namespace tincup
