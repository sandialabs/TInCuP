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
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <typeindex>
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
                  "Error: Function must be callable with an argument of type std::bool_constant<true>");
    if (value) {
      return std::forward<F>(f)(std::bool_constant<true>{});
    } else {
      return std::forward<F>(f)(std::bool_constant<false>{});
    }
  }

  const bool value;
}; // struct BoolDispatch

} // namespace tincup



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
  for(size_t i = 0; i < N; ++i) {
    if (lhs.value[i] != rhs.value[i]) return false;
  }
  return true;
}

template<std::size_t N, std::size_t M>
constexpr bool operator == ( const StringLiteral<N>& lhs, 
		             const char (&rhs)[M] ) {
  if constexpr (N != M) return false;
  for(size_t i = 0; i < N; ++i) {
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

// Macro to declare CPO metadata - compile-time string literal for powerful metaprogramming
#define TINCUP_CPO_TAG(name_str) \
  static constexpr auto name = ::tincup::StringLiteral{name_str}; \
  static constexpr auto qualified_name() noexcept { \
    return ::tincup::StringLiteral{"tincup::" name_str}; \
  } \
  static constexpr auto descriptor() noexcept { \
    return ::tincup::cpo_descriptor<name>(); \
  }

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

template<auto LhsValue, typename Rhs>
constexpr bool operator == ( constant_t<LhsValue>, 
		             const Rhs& rhs ) {
  return LhsValue == rhs;
}

template<typename Lhs, auto RhsValue>
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

namespace tincup {
// Low-cost diagnostic utilities - avoid expensive permutation checking
namespace detail {
  template<typename Derived, typename... Args>
  constexpr bool check_binary_swap() {
    // HARD LIMIT: Only check binary operations to avoid factorial explosion
    if constexpr (sizeof...(Args) == 2) {
      return []<typename T1, typename T2>(std::type_identity<T1>, std::type_identity<T2>) {
        // Only check if swapped (T2, T1) instead of (T1, T2) works
        return requires {
          tag_invoke_cpo(std::declval<const Derived&>(), 
                        std::declval<T2>(), 
                        std::declval<T1>()); 
        };
      }(std::type_identity<Args>{}...);
    }
    return false; // NO checking for arity > 2 (too expensive)
  }

  // Check for common arity mistakes (low cost - just a few additional checks)
  template<typename Derived, typename... Args>
  constexpr bool check_common_arities() {
    constexpr std::size_t current_arity = sizeof...(Args);

    // Only check a few common cases to avoid explosion
    if constexpr (current_arity == 1) {
      // Maybe they meant to call a binary CPO?
      return []<typename T>(std::type_identity<T>) {
        using ArgType = T;
        return requires { tag_invoke_cpo(std::declval<const Derived&>(), std::declval<ArgType>(), std::declval<ArgType>()); };
      }(std::type_identity<Args>{}...);
    } else if constexpr (current_arity == 3) {
      // Maybe they meant a binary CPO (common mistake in generic code)?
      return []<typename T1, typename T2, typename T3>(std::type_identity<T1>, std::type_identity<T2>, std::type_identity<T3>) {
        return requires { tag_invoke_cpo(std::declval<const Derived&>(), std::declval<T1>(), std::declval<T2>()); } ||
               requires { tag_invoke_cpo(std::declval<const Derived&>(), std::declval<T2>(), std::declval<T3>()); };
      }(std::type_identity<Args>{}...);
    }
    return false;
  }
}

} // namespace tincup

// Enhanced diagnostic opt-out controls for performance-conscious users
// Define any of these macros to disable specific diagnostic categories:
//   TINCUP_DISABLE_POINTER_DIAGNOSTICS   - Skip deref_if_needed checks
//   TINCUP_DISABLE_CONST_DIAGNOSTICS     - Skip const_cast_if_needed checks
//   TINCUP_DISABLE_ORDER_DIAGNOSTICS     - Skip binary argument swap checks
//   TINCUP_DISABLE_ARITY_DIAGNOSTICS     - Skip wrong argument count checks
//   TINCUP_DISABLE_ALL_DIAGNOSTICS       - Disable all enhanced diagnostics
//   TINCUP_MINIMAL_DIAGNOSTICS           - Same as DISABLE_ALL (alias for clarity)
// Or select with a single knob:
//   TINCUP_DIAGNOSTIC_LEVEL in {0,1,2,3}
//     0: disable all (alias of DISABLE_ALL)
//     1: keep pointer/const, disable order/arity
//     2: keep pointer/const/order, disable arity
//     3: enable all (default behavior)

// Single-knob mapping
#if defined(TINCUP_DIAGNOSTIC_LEVEL)
  #if (TINCUP_DIAGNOSTIC_LEVEL == 0)
    #ifndef TINCUP_DISABLE_ALL_DIAGNOSTICS
      #define TINCUP_DISABLE_ALL_DIAGNOSTICS
    #endif
  #elif (TINCUP_DIAGNOSTIC_LEVEL == 1)
    #ifndef TINCUP_DISABLE_ORDER_DIAGNOSTICS
      #define TINCUP_DISABLE_ORDER_DIAGNOSTICS
    #endif
    #ifndef TINCUP_DISABLE_ARITY_DIAGNOSTICS
      #define TINCUP_DISABLE_ARITY_DIAGNOSTICS
    #endif
  #elif (TINCUP_DIAGNOSTIC_LEVEL == 2)
    #ifndef TINCUP_DISABLE_ARITY_DIAGNOSTICS
      #define TINCUP_DISABLE_ARITY_DIAGNOSTICS
    #endif
  #else
    // Level 3 or any other value: full diagnostics (no additional defines)
  #endif
#endif

#ifdef TINCUP_DISABLE_ALL_DIAGNOSTICS
  #define TINCUP_DISABLE_POINTER_DIAGNOSTICS
  #define TINCUP_DISABLE_CONST_DIAGNOSTICS
  #define TINCUP_DISABLE_ORDER_DIAGNOSTICS
  #define TINCUP_DISABLE_ARITY_DIAGNOSTICS
#endif

#ifdef TINCUP_MINIMAL_DIAGNOSTICS
  #define TINCUP_DISABLE_POINTER_DIAGNOSTICS
  #define TINCUP_DISABLE_CONST_DIAGNOSTICS
  #define TINCUP_DISABLE_ORDER_DIAGNOSTICS
  #define TINCUP_DISABLE_ARITY_DIAGNOSTICS
#endif

namespace tincup {

template<typename...>
inline constexpr bool always_false_v = false;

/**
 * @class cpo_diagnostic
 * @brief Provides some compile time checks of likely CPO common cases
 */
template<typename Derived>
class cpo_diagnostics {
protected:
	
  template<typename... Args>
  struct show_argument_types {
    static_assert(always_false_v<Args...>,
      "ARGUMENT TYPES: Inspect the template instantiation above to see actual argument types");
  };

  template<typename T>
  static constexpr bool 
  might_need_deref_v = has_deref_c<T> &&
                       !std::is_reference_v<T> && 
		       !std::is_same_v<std::remove_cvref_t<T>, deref_t<T>>;

// C++26 user-generated static_assert message support
#if __cpp_static_assert >= 202306L
  // Message structure for C++26 user-generated static_assert  
  struct diagnostic_message {
    constexpr diagnostic_message(std::string_view msg) : view(msg) {}
    constexpr std::size_t size() const { return view.size(); }
    constexpr const char* data() const { return view.data(); }
    std::string_view view;
  };

  template<typename CPO>
  static constexpr auto make_cpo_error_message(
    std::string_view context = "No valid tag_invoke overload"
  ) {
    if constexpr (requires { CPO::descriptor(); }) {
      constexpr auto name_view = CPO::descriptor().view();
      // In a real implementation, we could construct more dynamic messages
      return diagnostic_message{name_view};
    } else {
      return diagnostic_message{"CPO"};
    }
  } 
#endif

  // Enhanced diagnostic helper - reduces boilerplate in generated CPOs
  template<typename... Args>
  constexpr void enhanced_fail(Args&&... args) const {
	  
    // Try various combinations to provide helpful diagnostics (respecting opt-out flags)
    const auto& derived_cpo = static_cast<const Derived&>(*this);

#ifndef TINCUP_DISABLE_POINTER_DIAGNOSTICS
  constexpr bool deref_works = requires { 
    tag_invoke_cpo(derived_cpo, 
		   deref_if_needed(std::forward<Args>(args))...); 
  };
#else
    constexpr bool deref_works = false;
#endif

#ifndef TINCUP_DISABLE_CONST_DIAGNOSTICS
    constexpr bool unconst_works = requires { 
      tag_invoke_cpo(derived_cpo, const_cast_if_needed(std::forward<Args>(args))...); 
    };
    constexpr bool both_works = requires { 
      tag_invoke_cpo(derived_cpo, const_cast_if_needed(deref_if_needed(std::forward<Args>(args)))...);     };
#else
      constexpr bool unconst_works = false;
      constexpr bool both_works = false;
#endif

#ifndef TINCUP_DISABLE_ORDER_DIAGNOSTICS
      constexpr bool binary_swap_works = detail::check_binary_swap<Derived, Args...>();
#else
      constexpr bool binary_swap_works = false;
#endif

#ifndef TINCUP_DISABLE_ARITY_DIAGNOSTICS
      constexpr bool arity_mismatch = detail::check_common_arities<Derived, Args...>();
#else
      constexpr bool arity_mismatch = false;
#endif

      if constexpr (deref_works) {
#if __cpp_static_assert >= 202306L
        // C++26: User-generated static_assert with CPO name
        static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>());
#elif __cplusplus >= 202302L
        // C++23: Enhanced pointer diagnostic (no dynamic concatenation)
        static_assert(always_false_v<Derived>,
          "CPO [C++23]: No valid tag_invoke for this CPO, but valid for dereferenced types. "
          "Some arguments appear to be pointers/smart_ptrs needing dereferencing. "
          "Check template instantiation context above for specific argument types.");
#else
        // C++20: Pointer diagnostic with CPO name
        static_assert(always_false_v<Derived>,
          "CPO: No valid tag_invoke overload for CPO, but there IS a valid "
          "overload for the dereferenced arguments. Some arguments appear to be pointers/smart_ptrs "
          "that may need explicit dereferencing. Consider: cpo(*ptr) instead of cpo(ptr)");
#endif
      } else if constexpr (unconst_works) {
#if __cpp_static_assert >= 202306L
        // C++26: User-generated static_assert with CPO name
        static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>());
#elif __cplusplus >= 202302L
        // C++23: Enhanced const diagnostic
        static_assert(always_false_v<Derived>,
          "CPO [C++23]: No valid tag_invoke for this CPO, but valid for non-const types. "
        "Some arguments may have incorrect const-qualification. "
          "Check template instantiation context above - you may be passing const objects to mutating operations.");
#else
        // C++20: Const diagnostic
        static_assert(always_false_v<Derived>,
          "CPO: No valid tag_invoke overload for this CPO, but there IS a valid "
          "overload for non-const arguments. You may be passing const objects to a mutating operation. "
          "Consider: cpo(non_const_obj) instead of cpo(const_obj)");
#endif
      } else if constexpr (both_works) {
#if __cpp_static_assert >= 202306L
        // C++26: User-generated static_assert with CPO name
        static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>());
#elif __cplusplus >= 202302L
        // C++23: Enhanced combined diagnostic
        static_assert(always_false_v<Derived>,
          "CPO [C++23]: No valid tag_invoke for this CPO, but valid for dereferenced AND non-const types. "
          "Arguments may need both dereferencing and const removal. "
          "Check template instantiation context above for CPO and argument types.");
#else
        // C++20: Combined diagnostic
        static_assert(always_false_v<Derived>,
         "CPO: No valid tag_invoke overload for this CPO, but there IS a valid "
          "overload for dereferenced non-const arguments. You may need both pointer dereferencing "
          "and to remove const-qualification. Consider: cpo(*non_const_ptr) instead of cpo(const_ptr)");
#endif
      } else if constexpr (binary_swap_works) {
#if __cpp_static_assert >= 202306L
        // C++26: User-generated static_assert with CPO name
        static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>());
#elif __cplusplus >= 202302L
        // C++23: Enhanced argument order diagnostic
        static_assert(always_false_v<Derived>,
          "CPO [C++23]: No valid tag_invoke for this CPO, but valid for reordered arguments. "
          "Arguments may be in wrong order. Check template instantiation context above for expected argument order.");
#else
        // C++20: Argument order diagnostic
        static_assert(always_false_v<Derived>,
          "CPO: No valid tag_invoke overload for this CPO, but there IS a valid "
          "overload with reordered arguments. You may have swapped argument positions. "
          "Check the expected argument order for this CPO.");
#endif
      } else if constexpr (arity_mismatch) {
#if __cpp_static_assert >= 202306L
        // C++26: User-generated static_assert with CPO name
        static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>());
#elif __cplusplus >= 202302L
        // C++23: Enhanced arity diagnostic
        static_assert(always_false_v<Derived>,
       "CPO [C++23]: No valid tag_invoke for this CPO with the current argument count, but valid for a different arity. "
          "Wrong number of arguments provided. Check template instantiation context above for expected arity.");
#else
       // C++20: Arity diagnostic
        static_assert(always_false_v<Derived>,
          "CPO: No valid tag_invoke overload for this number of arguments, but there IS a valid "
          "overload with different arity. You may be passing too many or too few arguments. "
          "Check the expected number of arguments for this CPO.");
#endif
      } else {
        // Fallback to basic diagnostic with argument type display
        [[maybe_unused]] show_argument_types<Args...> display_types{};
 #if __cpp_static_assert >= 202306L
        // C++26: User-generated static_assert with CPO name (fallback case)
        static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>());
#else
        static_assert(always_false_v<Derived>,
          "CPO: No valid tag_invoke overload found. Check CPO and argument types above. "
          "Expected: auto tag_invoke(cpo_ftor, ...)");
#endif
    }
  } // 
}; // class cpo_diagnostics 

} // namespace tincup



template<typename...Args>
auto tag_invoke(Args&&...);

namespace tincup {

template<typename Derived>
struct cpo_introspection {

  template<typename...Args>                                                                            
  static constexpr bool valid_arg_types = requires { 
    tag_invoke(std::declval<Derived>(), 
	       std::declval<Args>()...); 
  };

  template<typename...Args>                                                                            
  static constexpr bool is_nothrow = requires { 
    { tag_invoke(std::declval<Derived>(), 
		 std::declval<Args>()...) } noexcept; 
  };

  template<typename...Args>                                                                            
  using return_type = decltype(tag_invoke(std::declval<Derived>(), 
			                  std::declval<Args>()...));

  // Clean alias for return types - eliminates typename/template keywords in generated code
  template<typename...Args>                                                                            
  using result_t = decltype(tag_invoke(std::declval<Derived>(), 
			               std::declval<Args>()...));

  template<template<class> typename Predicate, typename...Args>
  static constexpr bool valid_return_type = Predicate<return_type<Args...>>::value;
}; // cpo_introspection

} // namespace tincup

namespace tincup {

// CRTP base class for all CPOs                                                      
template<typename Derived> 
struct cpo_base : public cpo_introspection<Derived>,
                  public cpo_diagnostics<Derived> {
  // Catch-all fallback using immediate context SFINAE - only matches when derived has no operator()
  // This uses C++20 immediate context rules to avoid circular dependency
  template<typename... Args>
    requires (!std::invocable<Derived,Args...>)
    constexpr void operator()(Args&&... args) const {
    this->enhanced_fail(std::forward<Args>(args)...);
  }
}; // struct cpo_base  
	
} // namespace tincup



namespace tincup {

template<typename T>
concept cpo_c = std::is_trivially_constructible_v<T> &&
                std::is_empty_v<T>                   &&
                std::derived_from<T,cpo_base<T>>;

template<typename Cp, typename... Args>
requires cpo_c<Cp>
using cpo_return_t = typename Cp::template return_type<Args...>;

// Useful return type concepts
template<typename Cp, typename... Args>
concept returns_void_c = cpo_c<Cp> && Cp::template valid_return_type<std::is_void,Args...>;

template<typename Cp, typename... Args>
concept returns_integral_c = cpo_c<Cp> && Cp::template valid_return_type<std::is_integral,Args...>;

// Non-void return type
template<typename Cp, typename... Args>
concept returns_value_c = cpo_c<Cp> && (!Cp::template valid_return_type<std::is_void,Args...>);

} // namespace tincup



/**
 * @file type_list.hpp
 * @brief Defines compile time type container and associated template metaprogramming utilities
 */

/**
 * @file type_utils.hpp
 * @brief Collection of simple template metaprogramming components 
 */

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
 * @class Increment
 * @brief A template class for incrementing compile-time values.
 * @tparam T The type to be incremented.
 * @details This class template provides a mechanism to increment compile-time
 * values, particularly useful for template metaprogramming.
 */
template<class> struct Increment;

/**
 * @class Increment<T<I>>
 * @brief Specialization of Increment for template types with a size_t parameter.
 * @tparam T The template class to be incremented.
 * @tparam I The current value to be incremented.
 * @details This specialization increments the value I by 1 in the resulting type.
 */
template<template<std::size_t> class T, std::size_t I> 
struct Increment<T<I>> {
  using type = T<I+1>;
};

/**
 * @typedef increment_t
 * @brief Convenience alias for accessing the incremented type.
 * @tparam T The type to be incremented.
 * @details This alias template provides a shorthand for accessing the
 * incremented type defined by the Increment class template.
 */
template<class T>
using increment_t = typename Increment<T>::type;

} // namespace tincup

namespace tincup {

/**
 * @brief A helper type trait for finding the index of a given type in variadic parameter pack
 */
template<class, class...>
struct IndexOf;

template<class First, class Second, class... Rest>
struct IndexOf<First, Second, Rest...> : std::conditional_t<std::is_same_v<std::remove_cvref_t<First>, std::remove_cvref_t<Second>>,
                                                            size_constant<0>,
                                                            size_constant<1 + IndexOf<First, Rest...>::value>> {};

template<class T>
struct IndexOf<T> : size_constant<0> {};

template<class...> struct TypeList;

namespace detail {
template<class> 
struct IsTypeList : std::false_type {};

template<class...Ts>
struct IsTypeList<TypeList<Ts...>> : std::true_type {};
};

/**
 * @concept type_list_c
 * @brief Concept for types that satisfy the TypeList interface.
 */
template<class TL>
concept type_list_c = detail::IsTypeList<TL>::value;

template<class TL>
concept unique_type_list_c = type_list_c<TL> && TL::is_unique;

template<class TL>
concept nonempty_type_list_c = type_list_c<TL> && (TL::size > 0);
                             

/**
 * @class TypeListElement
 * @brief Retrieves the type at a specific index in a TypeList.
 *
 * @tparam I The index to retrieve.
 * @tparam TL The TypeList to retrieve from.
 */
template<std::size_t, type_list_c>
struct TypeListElement;

/**
 * @class TypeListElement<0u,TypeList<First,Rest...>> 
 * @brief Specialization for retrieving the first element of a TypeList.
 *
 * @tparam First The first type in the list.
 * @tparam Rest The remaining types in the list.
 */
template<class First, class...Rest>
struct TypeListElement<0u,TypeList<First,Rest...>> {
  using type = First;
};

/**
 * @class TypeListElement<I,TypeList<First,Rest...>> 
 * @brief Specialization for retrieving a non-first element of a TypeList.
 *
 * @tparam I The index to retrieve.
 * @tparam First The first type in the list.
 * @tparam Rest The remaining types in the list.
 */
template<std::size_t I, class First, class...Rest>
requires (I > 0 && I <= sizeof...(Rest))
struct TypeListElement<I,TypeList<First,Rest...>> {
  using type = typename TypeListElement<I-1u,TypeList<Rest...>>::type;
};

/**
 * @class TypeListElement<I,TypeList<Ts...>> 
 * @brief Specialization for handling out-of-bounds indices.
 *
 * @tparam I The index to retrieve.
 * @tparam Ts The types in the list.
 */
template<std::size_t I, class...Ts>
requires (I > sizeof...(Ts))
struct TypeListElement<I,TypeList<Ts...>> {
  // This will cause a compile-time error when accessed
  static_assert(I <= sizeof...(Ts), "Index out of bounds in TypeList");
};
// Note: pop_front is available as a member alias on TypeList

/**
 * @brief Concatenates an arbitrary number of TypeLists.
 *
 * @tparam Lists The TypeLists to concatenate.
 */
template<type_list_c... Lists>
struct ConcatenateTypeLists;

/**
 * @brief Base case: single TypeList.
 *
 * @tparam Ts Types in the single TypeList.
 */
template<class... Ts>
struct ConcatenateTypeLists<TypeList<Ts...>> {
  using type = TypeList<Ts...>;
};

/**
 * @brief Recursive case: two or more TypeLists.
 *
 * @tparam Ts Types in the first TypeList.
 * @tparam Us Types in the second TypeList.
 * @tparam Rest Remaining TypeLists.
 */
template<class... Ts, class... Us, type_list_c... Rest>
struct ConcatenateTypeLists<TypeList<Ts...>, TypeList<Us...>, Rest...> {
  using type = typename ConcatenateTypeLists<TypeList<Ts..., Us...>, Rest...>::type;
};

/**
 * @brief Alias template for concatenating multiple TypeLists.
 *
 * @tparam Lists The TypeLists to concatenate.
 */
template<type_list_c... Lists>
using concatenate_type_lists = typename ConcatenateTypeLists<Lists...>::type;

/**
 * @class TypeList
 * @brief A list of types with various utility operations.
 *
 * @tparam Ts The types in the list.
 */
template<class First, class...Rest>
struct TypeList<First,Rest...> {

  /// Number of types in the list
  static constexpr std::size_t size = 1 + sizeof...(Rest);

  using first_type = First;

  /**
   * @brief Appends a type to the end of the list.
   *
   * @tparam T The type to append.
   */
  template<class T>
  using append = TypeList<First,Rest...,T>;

  /**
   * @brief Retrieves the type at a specific index.
   *
   * @tparam I The index to retrieve.
   */
  template<std::size_t I>
  using type = typename TypeListElement<I, TypeList<First,Rest...>>::type;

  template<std::size_t I>
  requires (I<size)
  static auto get_type_index( size_constant<I> ) {
    return std::type_index(typeid(type<I>));
  }

   /**
   * @brief Removes the first type from the list.
   */
  using pop_front = TypeList<Rest...>;

  /**
   * @brief Adds a type to the front of the list.
   *
   * @tparam T The type to add.
   */
  template<class T>
  using push_front = TypeList<T,First,Rest...>;

  /**
   * @brief Determines if the type T is in the TypeList
   *
   * @tparam T The type to search for.
   *
   * @return constexpr bool True if T is one of the types in the TypeList
   */
  template<class T>
  static constexpr bool contains_type = std::disjunction_v<std::is_same<std::remove_cvref_t<T>,std::remove_cvref_t<First>>,std::is_same<std::remove_cvref_t<T>,std::remove_cvref_t<Rest>>...>;

  /**
   * @brief Finds the index of a type in the list if it exists
   *
   * @tparam T The type to search for.
   */
  template<class T>
  requires contains_type<T>
  static constexpr std::size_t index_of = IndexOf<T,First,Rest...>::value;

  /**
   * @brief Helper function to deduce if there are no repeated types.
   */
  template<std::size_t I = 0, std::size_t J = I + 1>
  static constexpr bool is_unique_impl() {
    if constexpr (I == size) {
      return true;
    } else if constexpr (J == size) {
      return is_unique_impl<I + 1, I + 2>();
    } else {
      return !std::is_same_v<type<I>, type<J>> && is_unique_impl<I, J + 1>();
    }
  }

  static constexpr bool is_unique = is_unique_impl();
};

template<>
struct TypeList<> {
  static constexpr std::size_t size = 0;

  template<class T>
  using append = TypeList<T>;

  template<class T>
  using push_front = TypeList<T>;

  template<class>
  static constexpr bool contains_type = false;

  static constexpr bool is_unique = true;
};

/**
 * @brief Specialization of Increment for TypeLists
 * @tparam Ts The types in the TypeList
 * @note Requires that Ts be incrementable
 */
template<class First, class...Rest>
struct Increment<TypeList<First,Rest...>> {
  using type = TypeList<increment_t<First>,increment_t<Rest>...>;
};

template<>
struct Increment<TypeList<>> {};

/**
 * @class IndexedTypeList
 * @brief A utility class for creating type lists indexed by compile-time values.
 * @tparam T A template that takes a std::size_t parameter and produces a type.
 * @details This class provides a way to create type lists where each element
 * is generated by applying the template T to a sequence of indices.
 */
template<template<std::size_t> class T>
struct IndexedTypeList {
  /**
   * @brief Helper function to create a TypeList from an index sequence.
   * @tparam Is Parameter pack of indices.
   * @return A TypeList containing T<Is>... types.
   */
  template<std::size_t...Is> 
  static constexpr auto eval( std::index_sequence<Is...> ) -> TypeList<T<Is>...> {
    return TypeList<T<Is>...>{};
  }

  /**
   * @brief Creates a TypeList with N elements.
   * @tparam N The number of elements in the resulting TypeList.
   * @return A TypeList containing T<0>, T<1>, ..., T<N-1> types.
   */
  template<std::size_t N> 
  static constexpr auto eval( size_constant<N> ) {
    return eval( std::make_index_sequence<N>{} );
  }

  /**
   * @typedef type
   * @brief Alias for the resulting TypeList.
   * @tparam N The number of elements in the TypeList.
   */
  template<std::size_t N>
  using type = decltype( eval(size_constant<N>{}) );
};

/**
 * @typedef indexed_type_list_t
 * @brief Convenience alias for creating an indexed type list.
 * @tparam T A template that takes a std::size_t parameter and produces a type.
 * @tparam N The number of elements in the resulting TypeList.
 * @details This alias provides a shorthand for creating a TypeList with N elements,
 * where each element is generated by applying the template T to indices 0 to N-1.
 */
template<template<std::size_t> class T, std::size_t N>
using indexed_type_list_t = typename IndexedTypeList<T>::template type<N>;

} // namespace tincup

namespace tincup {

namespace detail {
  void tag_invoke(...) = delete;  // Lowest priority poison pill
}

struct tag_invoke_t {
  template <typename Tag, typename... Args>
  auto operator()(Tag tag, Args&&... args) const
  -> decltype(tag_invoke(std::forward<Tag>(tag), std::forward<Args>(args)...)) {
    using detail::tag_invoke;  // Poison visible
    return tag_invoke(std::forward<Tag>(tag), std::forward<Args>(args)...);
  }
};

// The actual CPO instance.
inline constexpr tag_invoke_t tag_invoke_cpo{};
// ==================================================

// Forward declarations for concepts needed by cpo_base
template<typename Cp, typename...Args>
concept tag_invocable_c = requires ( const Cp& cpo, Args&&...args ) {
  { tag_invoke(cpo,std::forward<Args>(args)...) };
};

// Alias for brevity
template<typename Cp, typename...Args>
concept invocable_c = tag_invocable_c<Cp, Args...>;

template<typename Cp, typename...Args>
concept nothrow_tag_invocable_c = requires ( const Cp& cpo, Args&&...args ) {
  { tag_invoke(cpo,std::forward<Args>(args)...) } noexcept;
};

// Alias for brevity
template<typename Cp, typename...Args>
concept nothrow_invocable_c = nothrow_tag_invocable_c<Cp, Args...>;

// Forward declaration for invocable_t
template<typename Cp, typename...Args>
using tag_invocable_t = decltype(tag_invoke(std::declval<Cp>(),std::declval<Args>()...));

// Alias for brevity
template<typename Cp, typename...Args>
using invocable_t = tag_invocable_t<Cp, Args...>;

namespace detail {

template<cpo_c Cp, typename...Args>
consteval bool is_tag_invocable() {
  return tag_invocable_c<Cp,Args...>;
}

template<cpo_c Cp, typename...Args>
consteval bool is_nothrow_tag_invocable() {
  return nothrow_tag_invocable_c<Cp,Args...>;
}

// Aliases for brevity
template<cpo_c Cp, typename...Args>
consteval bool is_invocable() {
  return tag_invocable_c<Cp,Args...>;
}

template<cpo_c Cp, typename...Args>
consteval bool is_nothrow_invocable() {
  return nothrow_tag_invocable_c<Cp,Args...>;
}
} // namespace detail

// Variable templates for tag_invoke detection
template<typename Cp, typename...Args>
constexpr bool is_tag_invocable_v = detail::is_tag_invocable<Cp,Args...>();

template<typename Cp, typename...Args>
constexpr bool is_nothrow_tag_invocable_v = detail::is_nothrow_tag_invocable<Cp,Args...>();

// Aliases for brevity
template<typename Cp, typename...Args>
constexpr bool is_invocable_v = detail::is_invocable<Cp,Args...>();

template<typename Cp, typename...Args>
constexpr bool is_nothrow_invocable_v = detail::is_nothrow_invocable<Cp,Args...>();

} // namespace tincup

namespace tincup {

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
template<cpo_c Cp, typename...Args>
struct cpo_traits {
  static constexpr bool invocable = is_invocable_v<Cp,Args...>;
  static constexpr bool nothrow_invocable = is_nothrow_invocable_v<Cp,Args...>;
  static constexpr std::size_t arity = sizeof...(Args);
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

  // TypeList views of the argument types
  using arg_types_list = TypeList<Args...>;
  using decayed_arg_types_list = TypeList<std::remove_cvref_t<Args>...>;
  
  template<typename T>
  struct remove_all_pointers { using type = T; };
  template<typename T>
  struct remove_all_pointers<T*> { using type = typename remove_all_pointers<T>::type; };
  template<typename T>
  using remove_all_pointers_t = typename remove_all_pointers<T>::type;
  using raw_arg_types_list = TypeList<remove_all_pointers_t<std::remove_cvref_t<Args>>...>;

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
  static constexpr std::uint64_t bit_if(bool v) { return v ? (std::uint64_t{1} << I) : 0ull; }

  static constexpr std::uint64_t det_values_mask = []{
    if constexpr (arity == 0) return 0ull;
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (0ull | ... | bit_if<Is>(det_is_value_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr std::uint64_t det_pointers_mask = []{
    if constexpr (arity == 0) return 0ull;
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (0ull | ... | bit_if<Is>(det_is_pointer_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr std::uint64_t det_lvalue_refs_mask = []{
    if constexpr (arity == 0) return 0ull;
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (0ull | ... | bit_if<Is>(det_is_lvalue_ref_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr std::uint64_t det_rvalue_refs_mask = []{
    if constexpr (arity == 0) return 0ull;
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (0ull | ... | bit_if<Is>(det_is_rvalue_ref_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr std::uint64_t det_lvalue_const_refs_mask = []{
    if constexpr (arity == 0) return 0ull;
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (0ull | ... | bit_if<Is>(det_is_lvalue_ref_v<Is> && det_is_const_qualified_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr std::uint64_t det_const_qualified_mask = []{
    if constexpr (arity == 0) return 0ull;
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      return (0ull | ... | bit_if<Is>(det_is_const_qualified_v<Is>));
    }(std::make_index_sequence<arity>{});
  }();
  static constexpr std::uint64_t det_forwarding_refs_mask = 0ull; // not reliably detectable

  // Public masks: prefer generator metadata when available
  static constexpr std::uint64_t values_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<std::uint64_t>(cpo_arg_traits<Cp, Args...>::values_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<std::uint64_t>(Cp::template arg_traits<Args...>::values_mask);
    else return det_values_mask;
  }();
  static constexpr std::uint64_t pointers_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<std::uint64_t>(cpo_arg_traits<Cp, Args...>::pointers_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<std::uint64_t>(Cp::template arg_traits<Args...>::pointers_mask);
    else return det_pointers_mask;
  }();
  static constexpr std::uint64_t lvalue_refs_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<std::uint64_t>(cpo_arg_traits<Cp, Args...>::lvalue_refs_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<std::uint64_t>(Cp::template arg_traits<Args...>::lvalue_refs_mask);
    else return det_lvalue_refs_mask;
  }();
  static constexpr std::uint64_t rvalue_refs_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<std::uint64_t>(cpo_arg_traits<Cp, Args...>::rvalue_refs_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<std::uint64_t>(Cp::template arg_traits<Args...>::rvalue_refs_mask);
    else return det_rvalue_refs_mask;
  }();
  static constexpr std::uint64_t lvalue_const_refs_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<std::uint64_t>(cpo_arg_traits<Cp, Args...>::lvalue_const_refs_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<std::uint64_t>(Cp::template arg_traits<Args...>::lvalue_const_refs_mask);
    else return det_lvalue_const_refs_mask;
  }();
  static constexpr std::uint64_t forwarding_refs_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<std::uint64_t>(cpo_arg_traits<Cp, Args...>::forwarding_refs_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<std::uint64_t>(Cp::template arg_traits<Args...>::forwarding_refs_mask);
    else return det_forwarding_refs_mask;
  }();
  static constexpr std::uint64_t const_qualified_mask = []{
    if constexpr (cpo_arg_traits<Cp, Args...>::available) return static_cast<std::uint64_t>(cpo_arg_traits<Cp, Args...>::const_qualified_mask);
    else if constexpr (has_generator_arg_traits_c<Cp, Args...>) return static_cast<std::uint64_t>(Cp::template arg_traits<Args...>::const_qualified_mask);
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
      return (std::size_t{0} + ... + (decayed_arg_types_list::template index_of<typename decayed_arg_types_list::template type<Is>> == Is ? std::size_t{1} : std::size_t{0}));
    }(std::make_index_sequence<arity>{});
  }();

  static constexpr std::size_t unique_raw_types_count = []{
    if constexpr (arity == 0) return std::size_t{0};
    else return []<std::size_t...Is>(std::index_sequence<Is...>) {
      using list = raw_arg_types_list;
      return (std::size_t{0} + ... + (list::template index_of<typename list::template type<Is>> == Is ? std::size_t{1} : std::size_t{0}));
    }(std::make_index_sequence<arity>{});
  }();

  static constexpr bool args_unique_decayed = decayed_arg_types_list::is_unique;
  static constexpr bool args_unique_raw = raw_arg_types_list::is_unique;
}; // struct cpo_traits 

} // namespace tincup



/**
TInCuP - Detection helpers for cpo_impl specializations

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

namespace tincup { template<typename CPO, typename T> struct cpo_impl; }

namespace tincup {

// Primary detection: does a trait specialization exist for (CPO, Target) that
// provides a static call(Target&, Args...)?
template<typename CPO, typename Target, typename... Args>
concept has_cpo_impl_for_c = requires(Target& t, Args&&... args) {
  { cpo_impl<std::decay_t<CPO>, std::remove_cvref_t<Target>>::call(t, std::forward<Args>(args)...) };
};

template<typename CPO, typename Target, typename... Args>
struct has_cpo_impl_for : std::bool_constant<has_cpo_impl_for_c<CPO, Target, Args...>> {};

template<typename CPO, typename Target, typename... Args>
inline constexpr bool has_cpo_impl_for_v = has_cpo_impl_for_c<CPO, Target, Args...>;

// Convenience: detect by principal (first) argument type of the CPO call.
// Uses cpo_traits to obtain the first argument type from (Args...).
template<typename CPO, typename... Args>
using principal_arg_t = std::remove_cvref_t<typename cpo_traits<CPO, Args...>::template arg_t<0>>;

template<typename CPO, typename... Args>
concept has_specialized_cpo_impl_c = has_cpo_impl_for_c<CPO, principal_arg_t<CPO, Args...>, Args...>;

template<typename CPO, typename... Args>
struct has_specialized_cpo_impl : std::bool_constant<has_specialized_cpo_impl_c<CPO, Args...>> {};

template<typename CPO, typename... Args>
inline constexpr bool has_specialized_cpo_impl_v = has_specialized_cpo_impl_c<CPO, Args...>;

} // namespace tincup

namespace tincup {

// Registry namespace for CPO discovery and iteration
namespace registry {
  // Forward declaration - will be populated by generated CPOs
  template<typename F>
  constexpr void for_each_cpo([[maybe_unused]] F&& f) {
    // Implementation will be added when we have CPO registration mechanism
    static_assert(std::is_invocable_v<F>, "CPO iterator function must be callable");
  }
  
  template<StringLiteral Name>
  constexpr bool has_cpo_v = false; // Will be specialized by registered CPOs
}

// Formatter-style extension point for third-party types:
// Users may specialize cpo_impl<CPO, T> in their own code to provide
// implementations for types they do not control (similar to std::formatter).
template<typename CPO, typename T>
struct cpo_impl; // primary declaration (no definition)

// ==================================================
// CPO Usage Guidance
// ==================================================
// For CPO introspection, use the existing tincup predicates and type aliases:
//
//   tincup::is_invocable_v<my_cpo_ftor, Args...>        // Check if invocable
//   tincup::is_nothrow_invocable_v<my_cpo_ftor, Args...> // Check if nothrow
//   tincup::invocable_t<my_cpo_ftor, Args...>           // Get return type  
//   tincup::cpo_traits<my_cpo_ftor, Args...>            // Full introspection
//
// Concepts can be generated on-demand when needed:
//   template<typename... Args>
//   concept my_cpo_invocable_c = tincup::is_invocable_v<my_cpo_ftor, Args...>;

// ==================================================
// Composability helpers
// compose(f, g, ...) creates a function object that applies left-to-right
// e.g. compose(f, g)(x) == g(f(x))
// Works with CPOs and any invocable objects.
// ==================================================
namespace detail {
  template <class F, class G>
  struct composed {
    F f;
    G g;

    template <class... Args>
    constexpr auto operator()(Args&&... args) const
      noexcept(noexcept(std::invoke(g, std::invoke(f, std::forward<Args>(args)...))))
      -> decltype(std::invoke(g, std::invoke(f, std::forward<Args>(args)...))) {
      return std::invoke(g, std::invoke(f, std::forward<Args>(args)...));
    }
  }; // struct composed
}

template <class F, class G>
constexpr auto compose(F&& f, G&& g) {
  using Fd = std::decay_t<F>;
  using Gd = std::decay_t<G>;
  return detail::composed<Fd, Gd>{static_cast<Fd>(std::forward<F>(f)), 
	                          static_cast<Gd>(std::forward<G>(g))};
}

template <class F1, class F2, class... Fs>
constexpr auto compose(F1&& f1, F2&& f2, Fs&&... fs) {
  return compose(std::forward<F1>(f1), 
		 compose(std::forward<F2>(f2), 
			 std::forward<Fs>(fs)...));
}

} // namespace tincup
