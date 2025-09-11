/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#ifdef COMPILER_EXPLORER
#include <https://raw.githubusercontent.com/sandialabs/TInCuP/main/single_include/tincup.hpp>
#else
#include "single_include/tincup.hpp"
#endif

#include <memory>
#include <cmath>
#include <utility>

struct normalize_ftor;

template <class T>
struct Vector {
    T x, y;
};

inline constexpr struct normalize_ftor final : tincup::cpo_base<normalize_ftor> {
  TINCUP_CPO_TAG("normalize")
  using tincup::cpo_base<normalize_ftor>::operator();
  inline static constexpr bool is_variadic = false;
  template<typename V>
  requires tincup::invocable_c<normalize_ftor, const V&> 
  constexpr auto operator()(const V& vec) const
  noexcept(tincup::nothrow_invocable_c<normalize_ftor, const V&>) 
  -> tincup::invocable_t<normalize_ftor, const V&> {
    return tag_invoke(*this, vec);
  }
} normalize;

template<typename T>
Vector<T> tag_invoke( normalize_ftor, const Vector<T>& vec ) {
  auto norm = std::sqrt(vec.x*vec.x + vec.y*vec.y);
  return {vec.x/norm,vec.y/norm};    
}

int main() {
  /* expected_error: CPO: No valid tag_invoke overload for CPO, but there IS a valid overload for the dereferenced arguments. Some arguments appear to be pointers/smart_ptrs that may need explicit dereferencing. Consider: cpo(*ptr) instead of cpo(ptr) */
  auto ptr = std::make_unique<Vector<double>>();
  normalize(ptr);  // expect enhanced diagnostics (dereference hint)

  const Vector<double> v{4, 8};
  auto n = normalize(v);  // OK
}
