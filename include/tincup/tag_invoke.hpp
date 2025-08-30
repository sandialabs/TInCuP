/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include "cpo_concepts.hpp"

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
