/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

/**
TInCuP - ADL shim + cpo_impl integration test using std::vector
*/

#include <vector>
#include <cassert>
#include <tincup/tincup.hpp>

namespace myproj {

// Define a generic CPO: add_in_place(y, x) mutates y += x elementwise
inline constexpr struct add_in_place_ftor final : tincup::cpo_base<add_in_place_ftor> {
  TINCUP_CPO_TAG("add_in_place")

  template<class V>
  constexpr auto operator()(V& y, const V& x) const {
    return tag_invoke(*this, y, x);
  }
} add_in_place;

// ADL-visible shim in the CPO's namespace, forwarding to tincup::cpo_impl
template<typename T, typename Alloc, typename... Args>
constexpr auto tag_invoke(add_in_place_ftor,
                          std::vector<T, Alloc>& y,
                          Args&&... args)
  noexcept(noexcept(tincup::cpo_impl<add_in_place_ftor, std::vector<T, Alloc>>
                      ::call(y, std::forward<Args>(args)...)))
  -> decltype(tincup::cpo_impl<add_in_place_ftor, std::vector<T, Alloc>>
                ::call(y, std::forward<Args>(args)...))
{
  return tincup::cpo_impl<add_in_place_ftor, std::vector<T, Alloc>>
           ::call(y, std::forward<Args>(args)...);
}

} // namespace myproj

// Formatter-style trait specialization for third-party types
namespace tincup {

template<typename T, typename Alloc>
struct cpo_impl<myproj::add_in_place_ftor, std::vector<T, Alloc>> {
  static void call(std::vector<T, Alloc>& y, const std::vector<T, Alloc>& x) {
    assert(y.size() == x.size());
    for (std::size_t i = 0; i < y.size(); ++i) {
      y[i] += x[i];
    }
  }
};

} // namespace tincup

int main() {
  using myproj::add_in_place;

  {
    std::vector<int> a{1,2,3};
    std::vector<int> b{4,5,6};
    add_in_place(a, b);
    assert((a == std::vector<int>{5,7,9}));
  }

  {
    std::vector<double> a{1.5, 2.5};
    std::vector<double> b{0.5, 1.5};
    add_in_place(a, b);
    assert((a == std::vector<double>{2.0, 4.0}));
  }

  return 0;
}

