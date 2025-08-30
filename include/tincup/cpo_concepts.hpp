/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once 

#include "cpo_base.hpp"

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
