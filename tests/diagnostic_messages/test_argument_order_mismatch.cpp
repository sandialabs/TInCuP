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

#include <string>

// This CPO has a specific argument order
inline constexpr struct ordered_op_ftor final : tincup::cpo_base<ordered_op_ftor> {
  TINCUP_CPO_TAG("ordered_op")
  using tincup::cpo_base<ordered_op_ftor>::operator();
  template<typename T, typename U>
  requires tincup::invocable_c<ordered_op_ftor, T&&, U&&> 
  constexpr auto operator()(T&& t, U&& u) const
  noexcept(tincup::nothrow_invocable_c<ordered_op_ftor, T&&, U&&>) 
  -> tincup::invocable_t<ordered_op_ftor, T&&, U&&> {
    return tag_invoke(*this, std::forward<T>(t), std::forward<U>(u));
  }
} ordered_op;

// The implementation expects (int, std::string)
void tag_invoke(ordered_op_ftor, int, std::string) {}

int main() {
    /* expected_error: CPO: No valid tag_invoke overload for this CPO, but there IS a valid overload with reordered arguments. You may have swapped argument positions. Check the expected argument order for this CPO. */
    int i = 10;
    std::string s = "hello";

    // This call should fail because the arguments are in the wrong order.
    ordered_op(s, i);
}
