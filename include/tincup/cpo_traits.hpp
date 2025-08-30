/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include "tag_invoke.hpp"

namespace tincup {

// Enhanced introspection helper for CPOs
template<typename Cp, typename...Args>
struct cpo_traits {

  static constexpr bool invocable = is_invocable_v<Cp,Args...>;
  static constexpr bool nothrow_invocable = is_nothrow_invocable_v<Cp,Args...>;
  static constexpr std::size_t arity = sizeof...(Args);
  using return_t = std::conditional_t<invocable, invocable_t<Cp,Args...>, void>;

  // Enhanced introspection capabilities
  static constexpr bool is_void_returning = std::is_same_v<return_t, void>;
  static constexpr bool is_const_invocable = invocable && std::is_const_v<Cp>;

  // Variadic parameter-pack metadata is not recoverable from instantiated Args...
  // Detect a CPO-provided flag (emitted by the generator) if available.
  template<typename T, typename = void>
  struct variadic_flag_is : std::false_type {};
  template<typename T>
  struct variadic_flag_is<T, std::void_t<decltype(T::is_variadic)>>
      : std::bool_constant<static_cast<bool>(T::is_variadic)> {};

  // Backward-compat: older generated CPOs might expose has_variadic_params
  template<typename T, typename = void>
  struct variadic_flag_has_params : std::false_type {};
  template<typename T>
  struct variadic_flag_has_params<T, std::void_t<decltype(T::has_variadic_params)>>
      : std::bool_constant<static_cast<bool>(T::has_variadic_params)> {};

  static constexpr bool is_variadic = variadic_flag_is<Cp>::value || 
	                              variadic_flag_has_params<Cp>::value;

  // Type classification helpers
  template<std::size_t I>
  using arg_t = std::tuple_element_t<I, std::tuple<Args...>>;

  // Check if all arguments are references
  static constexpr bool all_args_are_refs = (std::is_reference_v<Args> && ...);

  // Check if all arguments are const references  
  static constexpr bool all_args_are_const_refs = (std::is_const_v<std::remove_reference_t<Args>> && ...);

  // Function signature as string_view (for debugging/logging)
  static constexpr std::string_view signature_hint() {
    if constexpr (arity == 0) return "()";
    else if constexpr (arity == 1) return "(T)";
    else if constexpr (arity == 2) return "(T, U)";
    else return "(T, U, ...)";
  }
}; // struct cpo_traits 

} // namespace tincup
