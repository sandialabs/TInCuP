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

template<typename T>
auto const_cast_if_needed(T&& x) noexcept -> std::remove_const_t<std::remove_reference_t<T>>&
requires (std::is_lvalue_reference_v<T> && std::is_const_v<std::remove_reference_t<T>>) {
  return const_cast<std::remove_const_t<std::remove_reference_t<T>>&>(x);
}

template<typename T>
auto const_cast_if_needed(T&& x) noexcept -> T&&
requires (!std::is_lvalue_reference_v<T> || !std::is_const_v<std::remove_reference_t<T>>) {
  return std::forward<T>(x);
}

template<typename T>
using const_cast_t = decltype(const_cast_if_needed(std::declval<T>()));

template<typename T>
auto add_const_if_needed(T&& x) noexcept -> const std::remove_reference_t<T>&
requires (std::is_lvalue_reference_v<T> && !std::is_const_v<std::remove_reference_t<T>>) {
  return const_cast<const std::remove_reference_t<T>&>(x);
}

template<typename T>
auto add_const_if_needed(T&& x) noexcept -> T&&
requires (!std::is_lvalue_reference_v<T> || std::is_const_v<std::remove_reference_t<T>>) {
  return std::forward<T>(x);
}

template<typename T>
using add_const_t = decltype(add_const_if_needed(std::declval<T>()));

} // namespace tincup
