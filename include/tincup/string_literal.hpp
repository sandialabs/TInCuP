/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <type_traits>

namespace tincup {

template<std::size_t N>
struct StringLiteral {
  constexpr StringLiteral(const char (&str)[N]) {
    std::copy_n(str, N, value.data());
  }
  std::array<char, N> value;
};

template<std::size_t N>
StringLiteral(const char (&)[N]) -> StringLiteral<N>;


template<std::size_t N, std::size_t M>
constexpr bool operator == ( const StringLiteral<N>& lhs, 
		             const StringLiteral<M>& rhs ) {
  if constexpr (N != M) return false;
  for(std::size_t i = 0; i < N; ++i) {
    if (lhs.value[i] != rhs.value[i]) return false;
  }
  return true;
}

template<std::size_t N, std::size_t M>
constexpr bool operator == ( const StringLiteral<N>& lhs, 
		             const char (&rhs)[M] ) {
  if constexpr (N != M) return false;
  for(std::size_t i = 0; i < N; ++i) {
    if (lhs.value[i] != rhs[i]) return false;
  }
  return true;
}


template<typename>
struct is_string_literal : std::false_type {};

template<std::size_t N>
struct is_string_literal<StringLiteral<N>> : std::true_type {};

template<typename T>
constexpr bool is_string_literal_v = is_string_literal<T>::value;

template<typename T>
concept string_literal_c = is_string_literal_v<T>;

} // namespace tincup
