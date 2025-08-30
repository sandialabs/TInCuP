/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include <functional>

#include "bool_dispatch.hpp"
#include "string_dispatch.hpp"
#include "cpo_base.hpp"
#include "cpo_concepts.hpp"
#include "cpo_traits.hpp"

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
