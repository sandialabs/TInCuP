/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include "dereference.hpp"

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
          tag_invoke(std::declval<const Derived&>(), 
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
        return requires { tag_invoke(std::declval<const Derived&>(), std::declval<ArgType>(), std::declval<ArgType>()); };
      }(std::type_identity<Args>{}...);
    } else if constexpr (current_arity == 3) {
      // Maybe they meant a binary CPO (common mistake in generic code)?
      return []<typename T1, typename T2, typename T3>(std::type_identity<T1>, std::type_identity<T2>, std::type_identity<T3>) {
        return requires { tag_invoke(std::declval<const Derived&>(), std::declval<T1>(), std::declval<T2>()); } ||
               requires { tag_invoke(std::declval<const Derived&>(), std::declval<T2>(), std::declval<T3>()); };
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
#if __cpp_static_assert >= 202306L && __cplusplus >= 202302L
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
      // Return the context message which contains the detailed diagnostic
      return diagnostic_message{context};
    } else {
      return diagnostic_message{context};
    }
  } 
#endif

  // Advanced diagnostic analysis using cpo_traits
  template<typename... Args>
  static constexpr auto analyze_arguments() {
    if constexpr (sizeof...(Args) > 0) {
      using traits = cpo_traits<Derived, Args...>;
      
      struct analysis {
        static constexpr auto arity = traits::arity;
        static constexpr auto pointers_mask = traits::pointers_mask;
        static constexpr auto const_qualified_mask = traits::const_qualified_mask;
        static constexpr auto lvalue_refs_mask = traits::lvalue_refs_mask;
        static constexpr auto rvalue_refs_mask = traits::rvalue_refs_mask;
        
        // Count problematic arguments by category
        static constexpr std::size_t pointer_count = __builtin_popcountll(pointers_mask);
        static constexpr std::size_t const_count = __builtin_popcountll(const_qualified_mask);
        static constexpr std::size_t lvalue_ref_count = __builtin_popcountll(lvalue_refs_mask);
        static constexpr std::size_t rvalue_ref_count = __builtin_popcountll(rvalue_refs_mask);
        
        // Pattern detection
        static constexpr bool all_pointers = (pointer_count == arity);
        static constexpr bool all_const = (const_count == arity);
        static constexpr bool mixed_ref_types = (lvalue_ref_count > 0 && rvalue_ref_count > 0);
        static constexpr bool all_rvalue_refs = (rvalue_ref_count == arity);
        
        // First problematic argument indices (for targeted messages)
        static constexpr std::size_t first_pointer_arg = pointers_mask ? __builtin_ctzll(pointers_mask) : arity;
        static constexpr std::size_t first_const_arg = const_qualified_mask ? __builtin_ctzll(const_qualified_mask) : arity;
      };
      return analysis{};
    } else {
      struct empty_analysis {
        static constexpr std::size_t arity = 0;
      };
      return empty_analysis{};
    }
  }

  // Macro-based diagnostic helpers for C++20/23 (creates verbose template errors)
  #define TINCUP_DIAGNOSTIC_POINTER_ARG(CPO, ARG_INDEX, ARG_TYPE) \
    template<typename = void> struct CPO##_diagnostic_pointer_arg_##ARG_INDEX { \
      static_assert(always_false_v<ARG_TYPE>, \
        "POINTER ARGUMENT DETECTED: Argument " #ARG_INDEX " has type " #ARG_TYPE \
        " but this CPO expects a dereferenced type. Try using operator* on this argument."); \
    }

  #define TINCUP_DIAGNOSTIC_CONST_ARG(CPO, ARG_INDEX, ARG_TYPE) \
    template<typename = void> struct CPO##_diagnostic_const_arg_##ARG_INDEX { \
      static_assert(always_false_v<ARG_TYPE>, \
        "CONST ARGUMENT DETECTED: Argument " #ARG_INDEX " has type " #ARG_TYPE \
        " but this mutating CPO requires non-const access. Remove const qualification."); \
    }

  #define TINCUP_DIAGNOSTIC_MIXED_REFS(CPO, LVALUE_COUNT, RVALUE_COUNT) \
    template<typename = void> struct CPO##_diagnostic_mixed_refs { \
      static_assert(always_false_v<CPO>, \
        "MIXED REFERENCE TYPES: Found " #LVALUE_COUNT " lvalue refs and " #RVALUE_COUNT \
        " rvalue refs. This CPO expects consistent reference types."); \
    }

  // Enhanced diagnostic helper - reduces boilerplate in generated CPOs
  template<typename... Args>
  constexpr void enhanced_fail(Args&&... args) const {
	  
    // Advanced argument analysis using cpo_traits
    constexpr auto analysis = analyze_arguments<Args...>();
    
    // Try various combinations to provide helpful diagnostics (respecting opt-out flags)
    const auto& derived_cpo = static_cast<const Derived&>(*this);

#ifndef TINCUP_DISABLE_POINTER_DIAGNOSTICS
  constexpr bool deref_works = requires { 
    tag_invoke(derived_cpo, 
               deref_if_needed(std::forward<Args>(args))...); 
  };
#else
    constexpr bool deref_works = false;
#endif

#ifndef TINCUP_DISABLE_CONST_DIAGNOSTICS
    constexpr bool unconst_works = requires { 
      tag_invoke(derived_cpo, const_cast_if_needed(std::forward<Args>(args))...); 
    };
    constexpr bool both_works = requires { 
      tag_invoke(derived_cpo, const_cast_if_needed(deref_if_needed(std::forward<Args>(args)))...); };
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
#if __cpp_static_assert >= 202306L && __cplusplus >= 202302L
        // C++26: Enhanced pointer diagnostic with detailed argument analysis
        if constexpr (analysis.arity > 0) {
          if constexpr (analysis.all_pointers) {
            static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
              "ALL ARGUMENTS ARE POINTERS: All arguments need dereferencing. "
              "This CPO expects dereferenced values, not pointers/smart_ptrs. "
              "Try: cpo(*arg1, *arg2, ...)"));
          } else if constexpr (analysis.pointer_count == 1) {
            static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
              "SINGLE POINTER ARGUMENT: One argument needs dereferencing. "
              "This CPO expects dereferenced values. Try using operator* on the pointer/smart_ptr argument."));
          } else {
            static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
              "MULTIPLE POINTER ARGUMENTS: Some arguments need dereferencing. "
              "This CPO expects dereferenced values, not pointers/smart_ptrs. "
              "Try dereferencing the pointer arguments with operator*."));
          }
        } else {
          static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
            "No valid tag_invoke overload for this CPO, but there IS a valid "
            "overload for the dereferenced arguments. Some arguments appear to be pointers/smart_ptrs "
            "that may need explicit dereferencing. Consider: cpo(*ptr) instead of cpo(ptr)"));
        }
#elif __cplusplus >= 202302L
        // C++23: Enhanced pointer diagnostic (no dynamic concatenation)
        static_assert(always_false_v<Derived>,
          "CPO [C++23]: No valid tag_invoke for this CPO, but valid for dereferenced types. "
          "Some arguments appear to be pointers/smart_ptrs needing dereferencing. "
          "Check template instantiation context above for specific argument types.");
#else
        // C++20: Advanced pointer diagnostic with template instantiation details
        if constexpr (analysis.arity > 0 && analysis.pointer_count > 0) {
          // Trigger detailed template errors showing specific problematic arguments
          if constexpr (analysis.first_pointer_arg < analysis.arity) {
            using problematic_arg = typename cpo_traits<Derived, Args...>::template arg_t<analysis.first_pointer_arg>;
            [[maybe_unused]] TINCUP_DIAGNOSTIC_POINTER_ARG(Derived, analysis.first_pointer_arg, problematic_arg) trigger_error{};
          }
        }
        static_assert(always_false_v<Derived>,
          "CPO: No valid tag_invoke overload for CPO, but there IS a valid "
          "overload for the dereferenced arguments. Some arguments appear to be pointers/smart_ptrs "
          "that may need explicit dereferencing. Consider: cpo(*ptr) instead of cpo(ptr)");
#endif
      } else if constexpr (unconst_works) {
#if __cpp_static_assert >= 202306L && __cplusplus >= 202302L
        // C++26: Enhanced const diagnostic with detailed argument analysis
        if constexpr (analysis.arity > 0) {
          if constexpr (analysis.all_const) {
            static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
              "ALL ARGUMENTS ARE CONST: This mutating CPO requires non-const access to all arguments. "
              "Remove const qualifiers or use mutable references."));
          } else if constexpr (analysis.const_count == 1) {
            static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
              "SINGLE CONST ARGUMENT: One argument is const-qualified but this mutating CPO requires non-const access. "
              "Remove const qualification from the problematic argument."));
          } else {
            static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
              "MULTIPLE CONST ARGUMENTS: Some arguments are const-qualified but this mutating CPO requires non-const access. "
              "Remove const qualifiers from the problematic arguments."));
          }
        } else {
          static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
            "No valid tag_invoke overload for this CPO, but there IS a valid "
            "overload for non-const arguments. You may be passing const objects to a mutating operation. "
            "Consider: cpo(non_const_obj) instead of cpo(const_obj)"));
        }
#elif __cplusplus >= 202302L
        // C++23: Enhanced const diagnostic
        static_assert(always_false_v<Derived>,
          "CPO [C++23]: No valid tag_invoke for this CPO, but valid for non-const types. "
        "Some arguments may have incorrect const-qualification. "
          "Check template instantiation context above - you may be passing const objects to mutating operations.");
#else
        // C++20: Advanced const diagnostic with template instantiation details
        if constexpr (analysis.arity > 0 && analysis.const_count > 0) {
          // Trigger detailed template errors showing specific problematic arguments
          if constexpr (analysis.first_const_arg < analysis.arity) {
            using problematic_arg = typename cpo_traits<Derived, Args...>::template arg_t<analysis.first_const_arg>;
            [[maybe_unused]] TINCUP_DIAGNOSTIC_CONST_ARG(Derived, analysis.first_const_arg, problematic_arg) trigger_error{};
          }
        }
        static_assert(always_false_v<Derived>,
          "CPO: No valid tag_invoke overload for this CPO, but there IS a valid "
          "overload for non-const arguments. You may be passing const objects to a mutating operation. "
          "Consider: cpo(non_const_obj) instead of cpo(const_obj)");
#endif
      } else if constexpr (both_works) {
#if __cpp_static_assert >= 202306L && __cplusplus >= 202302L
        // C++26: Enhanced combined diagnostic with user-generated static_assert
        static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
          "No valid tag_invoke overload for this CPO, but there IS a valid "
          "overload for dereferenced non-const arguments. You may need both pointer dereferencing "
          "and to remove const-qualification. Consider: cpo(*non_const_ptr) instead of cpo(const_ptr)"));
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
#if __cpp_static_assert >= 202306L && __cplusplus >= 202302L
        // C++26: Enhanced argument order diagnostic with user-generated static_assert
        static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
          "No valid tag_invoke overload for this CPO, but there IS a valid "
          "overload with reordered arguments. You may have swapped argument positions. "
          "Check the expected argument order for this CPO."));
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
#if __cpp_static_assert >= 202306L && __cplusplus >= 202302L
        // C++26: Enhanced arity diagnostic with user-generated static_assert
        static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
          "No valid tag_invoke overload for this number of arguments, but there IS a valid "
          "overload with different arity. You may be passing too many or too few arguments. "
          "Check the expected number of arguments for this CPO."));
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
        // Advanced pattern-based diagnostics for special cases
        if constexpr (analysis.arity > 0) {
          if constexpr (analysis.mixed_ref_types) {
#if __cpp_static_assert >= 202306L && __cplusplus >= 202302L
            static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
              "MIXED REFERENCE TYPES: Arguments have inconsistent reference types (both lvalue and rvalue refs). "
              "This CPO expects consistent reference categories. Check your argument forwarding."));
#else
            [[maybe_unused]] TINCUP_DIAGNOSTIC_MIXED_REFS(Derived, analysis.lvalue_ref_count, analysis.rvalue_ref_count) trigger_error{};
#endif
          } else if constexpr (analysis.all_rvalue_refs) {
#if __cpp_static_assert >= 202306L && __cplusplus >= 202302L
            static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
              "ALL RVALUE REFERENCES: All arguments are rvalue references, but this CPO may expect lvalue references. "
              "Consider storing values in variables first, or check if you're using std::move() unnecessarily."));
#else
            static_assert(always_false_v<Derived>,
              "CPO: All arguments are rvalue references. This CPO may expect lvalue references or stored values. "
              "Consider: auto var = value; cpo(var); instead of cpo(std::move(value))");
#endif
          }
        }
        
        // Fallback to basic diagnostic with argument type display
        [[maybe_unused]] show_argument_types<Args...> display_types{};
 #if __cpp_static_assert >= 202306L && __cplusplus >= 202302L
        // C++26: Enhanced fallback diagnostic with user-generated static_assert
        static_assert(always_false_v<Derived>, make_cpo_error_message<Derived>(
          "No valid tag_invoke overload found. Check CPO and argument types above. "
          "Expected: auto tag_invoke(cpo_ftor, ...)"));
#else
        static_assert(always_false_v<Derived>,
          "CPO: No valid tag_invoke overload found. Check CPO and argument types above. "
          "Expected: auto tag_invoke(cpo_ftor, ...)");
#endif
    }
  } // 
}; // class cpo_diagnostics 
















} // namespace tincup
