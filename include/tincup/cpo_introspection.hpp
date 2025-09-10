/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include <type_traits>
#include <utility>

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
