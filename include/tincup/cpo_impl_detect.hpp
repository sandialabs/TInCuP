/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

/**
TInCuP - Detection helpers for cpo_impl specializations

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#pragma once

#include <type_traits>

#include "cpo_traits.hpp"

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
