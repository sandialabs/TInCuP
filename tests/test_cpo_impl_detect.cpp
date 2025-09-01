/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

/**
TInCuP - cpo_impl detection helpers test
*/

#include <vector>
#include <array>
#include <cassert>
#include <functional>
#include <algorithm>

#include <tincup/tincup.hpp>

namespace detect_test {

// Define a simple CPO: add_in_place(y, x)
inline constexpr struct add_in_place_ftor final : tincup::cpo_base<add_in_place_ftor> {
  TINCUP_CPO_TAG("add_in_place")

  template<class V>
  constexpr auto operator()(V& y, const V& x) const {
    return tag_invoke(*this, y, x);
  }
} add_in_place;

// Generic fallback for ranges – disabled when a cpo_impl specialization exists
template<std::ranges::range R>
  requires (!tincup::has_cpo_impl_for_c<add_in_place_ftor, R, R&, const R&>)
void tag_invoke(add_in_place_ftor, R& y, const R& x) {
  std::ranges::transform(y, x, std::ranges::begin(y), std::plus<>{});
}

// Forwarding shim for std::vector – enabled only when a cpo_impl specialization exists
template<typename T, typename Alloc>
  requires (tincup::has_cpo_impl_for_c<add_in_place_ftor,
                                       std::vector<T,Alloc>,
                                       std::vector<T,Alloc>&,
                                       const std::vector<T,Alloc>&>)
void tag_invoke(add_in_place_ftor, std::vector<T,Alloc>& y, const std::vector<T,Alloc>& x) {
  tincup::cpo_impl<add_in_place_ftor, std::vector<T,Alloc>>::call(y, x);
}

} // namespace detect_test

// Provide a trait specialization for std::vector that intentionally differs
// from the generic fallback so we can observe dispatch.
namespace tincup {
template<typename T, typename Alloc>
struct cpo_impl<detect_test::add_in_place_ftor, std::vector<T, Alloc>> {
  static void call(std::vector<T, Alloc>& y, const std::vector<T, Alloc>& x) {
    // y += 2*x (distinct from generic y += x)
    assert(y.size() == x.size());
    for (std::size_t i = 0; i < y.size(); ++i) {
      y[i] += 2 * x[i];
    }
  }
};
} // namespace tincup

int main() {
  using detect_test::add_in_place;

  // Vector path should use trait specialization (y += 2*x)
  {
    std::vector<int> y{4,5,6};
    std::vector<int> x{1,2,3};
    add_in_place(y, x);
    assert((y == std::vector<int>{6,9,12}));
  }

  // Array path should use generic fallback (y += x)
  {
    std::array<int,3> y{4,5,6};
    std::array<int,3> x{1,2,3};
    add_in_place(y, x);
    assert((y == std::array<int,3>{5,7,9}));
  }

  return 0;
}
