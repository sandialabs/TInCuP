/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include "cpo_diagnostics.hpp"
#include "cpo_introspection.hpp"

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
