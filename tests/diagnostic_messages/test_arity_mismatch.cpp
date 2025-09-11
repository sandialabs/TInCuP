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

// This CPO has a specific arity
inline constexpr struct two_arg_op_ftor final : tincup::cpo_base<two_arg_op_ftor> {
  TINCUP_CPO_TAG("two_arg_op")
  using tincup::cpo_base<two_arg_op_ftor>::operator();
  template<typename... Args>
  requires tincup::invocable_c<two_arg_op_ftor, Args&&...> 
  constexpr auto operator()(Args&&... args) const
  noexcept(tincup::nothrow_invocable_c<two_arg_op_ftor, Args&&...>) 
  -> tincup::invocable_t<two_arg_op_ftor, Args&&...> {
    return tag_invoke(*this, std::forward<Args>(args)...);
  }
} two_arg_op;

// The implementation expects two arguments
void tag_invoke(two_arg_op_ftor, int, double) {}

int main() {
    /* expected_error: CPO: No valid tag_invoke overload for this number of arguments, but there IS a valid overload with different arity. You may be passing too many or too few arguments. Check the expected number of arguments for this CPO. */
    // This call should fail because the number of arguments is wrong.
    two_arg_op(1);
}
