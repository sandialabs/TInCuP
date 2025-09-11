/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#include "single_include/tincup.hpp"

// This CPO expects a non-const reference
inline constexpr struct mutating_op_ftor final : tincup::cpo_base<mutating_op_ftor> {
  TINCUP_CPO_TAG("mutating_op")
  using tincup::cpo_base<mutating_op_ftor>::operator();
  template<typename V>
  requires tincup::invocable_c<mutating_op_ftor, V&> 
  constexpr auto operator()(V& vec) const
  noexcept(tincup::nothrow_invocable_c<mutating_op_ftor, V&>) 
  -> tincup::invocable_t<mutating_op_ftor, V&> {
    return tag_invoke(*this, vec);
  }
} mutating_op;

struct MyType { int value; };

// The implementation of the CPO takes a non-const reference
void tag_invoke(mutating_op_ftor, MyType& data) {
    data.value = 42;
}

int main() {
    /* expected_error: CPO: No valid tag_invoke overload for this CPO, but there IS a valid overload for non-const arguments. You may be passing const objects to a mutating operation. Consider: cpo(non_const_obj) instead of cpo(const_obj) */
    const MyType obj{10}; 
    // This call should fail because obj is const but the CPO expects a non-const reference.
    mutating_op(obj);
}
