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
#include <functional>
#include <string_view>
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
