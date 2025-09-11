/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include <array>
#include <cstdint>
#include <tuple>
#include "type_list.hpp"
#include <type_traits>
#include <string_view>

#include "tag_invoke.hpp"

namespace tincup {

// Configure maximum supported CPO arity and derive a compact mask type
#ifndef TINCUP_MAX_CPO_ARITY
#  define TINCUP_MAX_CPO_ARITY 16
#endif
static_assert(TINCUP_MAX_CPO_ARITY > 0 && TINCUP_MAX_CPO_ARITY <= 64, "TINCUP_MAX_CPO_ARITY must be in (0, 64]");

using arity_type = std::conditional_t<
    (TINCUP_MAX_CPO_ARITY <= 8),  std::uint8_t,
    std::conditional_t<
      (TINCUP_MAX_CPO_ARITY <= 16), std::uint16_t,
      std::conditional_t<
        (TINCUP_MAX_CPO_ARITY <= 32), std::uint32_t,
        std::uint64_t
      >>
  >;

// External extension point for generator-provided argument metadata
// Default: not available. Generators can specialize this for a given CPO.
template<typename Cp, typename...Args>
struct cpo_arg_traits {
  static constexpr bool available = false;
};

// Prefer generator-provided metadata when available
template<typename T, typename...As>
concept has_generator_arg_traits_c = requires { typename T::template arg_traits<As...>; };

// Enhanced introspection helper for CPOs
template<typename Cp, typename...Args>
struct cpo_traits {
  static constexpr bool invocable = is_invocable_v<Cp,Args...>;
  static constexpr bool nothrow_invocable = is_nothrow_invocable_v<Cp,Args...>;
  static constexpr std::size_t arity = sizeof...(Args);
  static_assert(arity <= TINCUP_MAX_CPO_ARITY, "CPO arity exceeds TINCUP_MAX_CPO_ARITY");
  using return_t = std::conditional_t<invocable, invocable_t<Cp,Args...>, void>;

  static constexpr bool is_void_returning = std::is_same_v<return_t, void>;

  // Variadic parameter-pack metadata is not recoverable from instantiated Args...
  // Detect a CPO-provided flag (emitted by the generator) if available.
  template<typename T, typename = void>
  struct variadic_flag_is : std::false_type {};
  template<typename T>
  struct variadic_flag_is<T, std::void_t<decltype(T::is_variadic)>>
    : std::bool_constant<static_cast<bool>(T::is_variadic)> {};

  static constexpr bool is_variadic = variadic_flag_is<Cp>::value;

  // Type classification helpers
  template<std::size_t I>
  using arg_t = std::tuple_element_t<I, std::tuple<Args...>>;

  using args_tuple = std::tuple<Args...>;
  template<std::size_t I>
  using decayed_arg_t = std::remove_cvref_t<arg_t<I>>;

  // type_list views of the argument types
  using arg_types_list = type_list<Args...>;
  using decayed_arg_types_list = type_list<std::remove_cvref_t<Args>...>;
  
  template<typename T>
  struct remove_all_pointers { using type = T; };
  template<typename T>
  struct remove_all_pointers<T*> { using type = typename remove_all_pointers<T>::type; };
  template<typename T>
  using remove_all_pointers_t = typename remove_all_pointers<T>::type;
  using raw_arg_types_list = type_list<remove_all_pointers_t<std::remove_cvref_t<Args>>...>;

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
  
  // ==================================================
  // Argument category flags and masks (per-index + bitmasks)
  // ==================================================
  enum : unsigned {
    flag_value            = 1u << 0,
    flag_pointer          = 1u << 1,
    flag_lvalue_ref       = 1u << 2,
    flag_rvalue_ref       = 1u << 3,
    flag_const_qualified  = 1u << 4,
    flag_forwarding_ref   = 1u << 5
    // Note: Forwarding references cannot be reliably detected from Args alone.
  };

  // Fallback detection helpers
  template<std::size_t I>
  static constexpr bool det_is_pointer_v = std::is_pointer_v<std::remove_reference_t<arg_t<I>>>;
  template<std::size_t I>
  static constexpr bool det_is_lvalue_ref_v = std::is_lvalue_reference_v<arg_t<I>>;
  template<std::size_t I>
  static constexpr bool det_is_rvalue_ref_v = std::is_rvalue_reference_v<arg_t<I>>;
  template<std::size_t I>
  static constexpr bool det_is_value_v = (!std::is_reference_v<arg_t<I>> && !det_is_pointer_v<I>);
  template<std::size_t I>
  static constexpr bool det_is_const_qualified_v = []{
    using A = std::remove_reference_t<arg_t<I>>;
    if constexpr (std::is_pointer_v<A>) {
      using P = std::remove_pointer_t<A>;
      return std::is_const_v<std::remove_reference_t<P>>;
    } else {
      return std::is_const_v<A>;
    }
  }();

  template<std::size_t I>
  static constexpr arity_type bit_if(bool v) { return v ? (arity_type{1} << I) : arity_type{0}; }

  static constexpr arity_type det_values_mask = []{
    if constexpr (arity == 0) return arity_type{0};
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (arity_type{0} | ... | bit_if<Is>(det_is_value_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr arity_type det_pointers_mask = []{
    if constexpr (arity == 0) return arity_type{0};
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (arity_type{0} | ... | bit_if<Is>(det_is_pointer_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr arity_type det_lvalue_refs_mask = []{
    if constexpr (arity == 0) return arity_type{0};
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (arity_type{0} | ... | bit_if<Is>(det_is_lvalue_ref_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr arity_type det_rvalue_refs_mask = []{
    if constexpr (arity == 0) return arity_type{0};
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (arity_type{0} | ... | bit_if<Is>(det_is_rvalue_ref_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr arity_type det_lvalue_const_refs_mask = []{
    if constexpr (arity == 0) return arity_type{0};
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (arity_type{0} | ... | bit_if<Is>(det_is_lvalue_ref_v<Is> && det_is_const_qualified_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr arity_type det_const_qualified_mask = []{
    if constexpr (arity == 0) return arity_type{0};
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (arity_type{0} | ... | bit_if<Is>(det_is_const_qualified_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr arity_type det_forwarding_refs_mask = arity_type{0}; // not reliably detectable

  // Public masks: prefer generator metadata when available
  static constexpr arity_type values_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<arity_type>(cpo_arg_traits<Cp, Args...>::values_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<arity_type>(Cp::template arg_traits<Args...>::values_mask);
    else return det_values_mask;
  }();
  static constexpr arity_type pointers_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<arity_type>(cpo_arg_traits<Cp, Args...>::pointers_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<arity_type>(Cp::template arg_traits<Args...>::pointers_mask);
    else return det_pointers_mask;
  }();
  static constexpr arity_type lvalue_refs_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<arity_type>(cpo_arg_traits<Cp, Args...>::lvalue_refs_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<arity_type>(Cp::template arg_traits<Args...>::lvalue_refs_mask);
    else return det_lvalue_refs_mask;
  }();
  static constexpr arity_type rvalue_refs_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<arity_type>(cpo_arg_traits<Cp, Args...>::rvalue_refs_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<arity_type>(Cp::template arg_traits<Args...>::rvalue_refs_mask);
    else return det_rvalue_refs_mask;
  }();
  static constexpr arity_type lvalue_const_refs_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<arity_type>(cpo_arg_traits<Cp, Args...>::lvalue_const_refs_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<arity_type>(Cp::template arg_traits<Args...>::lvalue_const_refs_mask);
    else return det_lvalue_const_refs_mask;
  }();
  static constexpr arity_type forwarding_refs_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<arity_type>(cpo_arg_traits<Cp, Args...>::forwarding_refs_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<arity_type>(Cp::template arg_traits<Args...>::forwarding_refs_mask);
    else return det_forwarding_refs_mask;
  }();
  static constexpr arity_type const_qualified_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<arity_type>(cpo_arg_traits<Cp, Args...>::const_qualified_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<arity_type>(Cp::template arg_traits<Args...>::const_qualified_mask);
    else return det_const_qualified_mask;
  }();

  // Convenience: flags as array (empty for arity==0)
  static constexpr auto make_flags_array() {
    if constexpr (arity == 0) {
      return std::array<unsigned, 0>{};
    } else {
      return []<std::size_t...Is>(std::index_sequence<Is...>) {
        return std::array<unsigned, sizeof...(Is)>{ ((
            ((values_mask           >> Is) & 1ull ? flag_value          : 0u) |
            ((pointers_mask         >> Is) & 1ull ? flag_pointer        : 0u) |
            ((lvalue_refs_mask      >> Is) & 1ull ? flag_lvalue_ref     : 0u) |
            ((rvalue_refs_mask      >> Is) & 1ull ? flag_rvalue_ref     : 0u) |
            ((const_qualified_mask  >> Is) & 1ull ? flag_const_qualified: 0u) |
            ((forwarding_refs_mask  >> Is) & 1ull ? flag_forwarding_ref : 0u)
        ))... };
      }(std::make_index_sequence<arity>{});
    }
  }
  static constexpr auto arg_flags_array = make_flags_array();

  // ==================================================
  // Unique type counts
  // ==================================================
  template<std::size_t I>
  using raw_arg_t = remove_all_pointers_t<decayed_arg_t<I>>;

  template<std::size_t I, std::size_t... Prev>
  static constexpr bool is_unique_decayed_index_impl(std::index_sequence<Prev...>) {
    if constexpr (I == 0) return true;
    else return (!(std::is_same_v<decayed_arg_t<I>, decayed_arg_t<Prev>> || ...));
  }

  template<std::size_t I, std::size_t... Prev>
  static constexpr bool is_unique_raw_index_impl(std::index_sequence<Prev...>) {
    if constexpr (I == 0) return true;
    else return (!(std::is_same_v<raw_arg_t<I>, raw_arg_t<Prev>> || ...));
  }

  // Unique-type metrics using TypeList utilities
  static constexpr std::size_t unique_decayed_types_count = []{
    if constexpr (arity == 0) return std::size_t{0};
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      // Count indices that are the first occurrence of their type
      return (std::size_t{0} + ... + (decayed_arg_types_list::template index_of_v<typename decayed_arg_types_list::template type<Is>> == Is ? std::size_t{1} : std::size_t{0}));
    }(std::make_index_sequence<arity>{});
  }();

  static constexpr std::size_t unique_raw_types_count = []{
    if constexpr (arity == 0) return std::size_t{0};
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      using list = raw_arg_types_list;
      return (std::size_t{0} + ... + (list::template index_of_v<typename list::template type<Is>> == Is ? std::size_t{1} : std::size_t{0}));
    }(std::make_index_sequence<arity>{});
  }();

  static constexpr bool args_unique_decayed = decayed_arg_types_list::is_unique;
  static constexpr bool args_unique_raw = raw_arg_types_list::is_unique;
}; // struct cpo_traits 

} // namespace tincup
