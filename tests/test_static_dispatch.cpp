/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#include <tincup/tincup.hpp>
#include <string>
#include <type_traits>

namespace test_static_dispatch {

struct Data {};

// Boolean dispatch CPO
inline constexpr struct choose_path_ftor final : tincup::cpo_base<choose_path_ftor> {
  TINCUP_CPO_TAG("choose_path")

  static constexpr struct yin_tag {} yin{};
  static constexpr struct yang_tag {} yang{};

  template<typename T>
  constexpr auto operator()(T& input, bool selector = false) const {
    tincup::BoolDispatch dispatcher(selector);
    return dispatcher.receive([&](auto dispatch_constant) {
      if constexpr (dispatch_constant.value) {
        return tag_invoke(*this, input, yin);
      } else {
        return tag_invoke(*this, input, yang);
      }
    });
  }

  template<typename T>
  constexpr auto operator()(T& input, yin_tag) const {
    return tag_invoke(*this, input, yin);
  }
  template<typename T>
  constexpr auto operator()(T& input, yang_tag) const {
    return tag_invoke(*this, input, yang);
  }
} choose_path{};

// String dispatch CPO
inline constexpr struct execute_policy_ftor final : tincup::cpo_base<execute_policy_ftor> {
  TINCUP_CPO_TAG("execute_policy")

  static constexpr struct fast_tag {} fast{};
  static constexpr struct safe_tag {} safe{};
  static constexpr struct debug_tag {} debug{};
  static constexpr struct not_found_tag {} not_found{};

  inline static constexpr auto options = tincup::string_view_array<3>{ "fast", "safe", "debug" };

  template<typename T>
  constexpr auto operator()(T& data, std::string_view policy_name) const {
    tincup::StringDispatch<3> dispatcher(policy_name, options);
    return dispatcher.receive([&](auto dispatch_constant) {
      if constexpr (dispatch_constant.value == 0) {
        return tag_invoke(*this, data, fast);
      } else if constexpr (dispatch_constant.value == 1) {
        return tag_invoke(*this, data, safe);
      } else if constexpr (dispatch_constant.value == 2) {
        return tag_invoke(*this, data, debug);
      } else {
        return tag_invoke(*this, data, not_found);
      }
    });
  }

  template<typename T>
  constexpr auto operator()(T& data, fast_tag) const { return tag_invoke(*this, data, fast); }
  template<typename T>
  constexpr auto operator()(T& data, safe_tag) const { return tag_invoke(*this, data, safe); }
  template<typename T>
  constexpr auto operator()(T& data, debug_tag) const { return tag_invoke(*this, data, debug); }
  template<typename T>
  constexpr auto operator()(T& data, not_found_tag) const { return tag_invoke(*this, data, not_found); }

} execute_policy{};

} // namespace test_static_dispatch

// tag_invoke definitions in the same namespace as the argument type (ADL)
namespace test_static_dispatch {

// For bool dispatch: return int in both branches for a consistent return type
inline int tag_invoke(choose_path_ftor, Data&, choose_path_ftor::yin_tag) { return 1; }
inline int tag_invoke(choose_path_ftor, Data&, choose_path_ftor::yang_tag) { return 2; }

// For string dispatch: return std::size_t for all paths
inline std::size_t tag_invoke(execute_policy_ftor, Data&, execute_policy_ftor::fast_tag) { return 10u; }
inline std::size_t tag_invoke(execute_policy_ftor, Data&, execute_policy_ftor::safe_tag) { return 20u; }
inline std::size_t tag_invoke(execute_policy_ftor, Data&, execute_policy_ftor::debug_tag) { return 30u; }
inline std::size_t tag_invoke(execute_policy_ftor, Data&, execute_policy_ftor::not_found_tag) { return 0u; }

} // namespace test_static_dispatch

// Simple compile-time check: calling with a temporary should be ill-formed
static_assert(!std::is_invocable_v<decltype(test_static_dispatch::choose_path), test_static_dispatch::Data, bool>);

int main() {
  using namespace test_static_dispatch;
  Data d{};

  // Bool dispatch compile/use checks
  int a = choose_path(d, true);
  int b = choose_path(d, false);
  int c = choose_path(d, choose_path_ftor::yin_tag{});
  int e = choose_path(d, choose_path_ftor::yang_tag{});

  // String dispatch compile/use checks
  std::size_t f = execute_policy(d, "fast");
  std::size_t g = execute_policy(d, execute_policy_ftor::safe_tag{});
  std::size_t h = execute_policy(d, "unknown"); // not_found path

  return (a + b + c + e + static_cast<int>(f + g + h)) == -1; // suppress unused warnings
}
