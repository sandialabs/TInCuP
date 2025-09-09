/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

/**
 * @file type_utils.hpp
 * @brief Collection of simple template metaprogramming components 
 */

#include <concepts>
#include <type_traits>
#include <utility>
// Intentionally header-only; keep dependencies minimal

namespace tincup {

/**
 * @class size_constant
 * @brief A type equivalent to a whole number.
 * @tparam N The compile-time constant value of type std::size_t.
 * @details
 * This trait provides a convenient way to create compile-time size constants.
 * It is equivalent to std::integral_constant<std::size_t, N>.
 *
 * @note size_constant<N>::value is a constant expression of type std::size_t with value N.
 * @note size_constant<N> is an empty, trivial type that can be used as a tag.
 *
 * @par Example:
 * @code
 * // Define a compile-time size constant
 * using Five = size_constant<5>;
 * 
 * // Use in a template function
 * template<typename T, typename N>
 * struct Array {
 *     T data[N::value];
 * };
 * 
 * // Usage
 * Array<int, Five> arr; // Array of 5 integers
 * 
 * // Use in a constexpr context
 * constexpr std::size_t ten = size_constant<10>::value;
 * @endcode
 *
 * @see std::integral_constant
 */
template<std::size_t N>
struct size_constant {
  static constexpr std::size_t value = N;
};

/**
 * @class increment
 * @brief A template class for incrementing compile-time values.
 * @tparam T The type to be incremented.
 * @details This class template provides a mechanism to increment compile-time
 * values, particularly useful for template metaprogramming.
 */
template<class> struct increment;

/**
 * @class increment<T<I>>
 * @brief Specialization of increment for template types with a size_t parameter.
 * @tparam T The template class to be incremented.
 * @tparam I The current value to be incremented.
 * @details This specialization increments the value I by 1 in the resulting type.
 */
template<template<std::size_t> class T, std::size_t I> 
struct increment<T<I>> {
  using type = T<I+1>;
};


/**
 * @typedef increment_t
 * @brief Convenience alias for accessing the incremented type.
 * @tparam T The type to be incremented.
 * @details This alias template provides a shorthand for accessing the
 * incremented type defined by the increment class template.
 */
template<class T>
using increment_t = typename increment<T>::type;

} // namespace tincup
