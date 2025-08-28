/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

#include <tincup/tincup.hpp>
#include <string>

namespace meson_test_static_dispatch {

struct Data {};

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
  constexpr auto operator()(T& input, yin_tag) const { return tag_invoke(*this, input, yin); }
  template<typename T>
  constexpr auto operator()(T& input, yang_tag) const { return tag_invoke(*this, input, yang); }
} choose_path{};

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
      if constexpr (dispatch_constant.value == 0) return tag_invoke(*this, data, fast);
      else if constexpr (dispatch_constant.value == 1) return tag_invoke(*this, data, safe);
      else if constexpr (dispatch_constant.value == 2) return tag_invoke(*this, data, debug);
      else return tag_invoke(*this, data, not_found);
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

inline int tag_invoke(choose_path_ftor, Data&, choose_path_ftor::yin_tag) { return 1; }
inline int tag_invoke(choose_path_ftor, Data&, choose_path_ftor::yang_tag) { return 2; }
inline std::size_t tag_invoke(execute_policy_ftor, Data&, execute_policy_ftor::fast_tag) { return 10u; }
inline std::size_t tag_invoke(execute_policy_ftor, Data&, execute_policy_ftor::safe_tag) { return 20u; }
inline std::size_t tag_invoke(execute_policy_ftor, Data&, execute_policy_ftor::debug_tag) { return 30u; }
inline std::size_t tag_invoke(execute_policy_ftor, Data&, execute_policy_ftor::not_found_tag) { return 0u; }

} // namespace meson_test_static_dispatch

int main() {
  using namespace meson_test_static_dispatch;
  Data d{};
  (void)choose_path(d, true);
  (void)choose_path(d, choose_path_ftor::yin_tag{});
  (void)execute_policy(d, "fast");
  (void)execute_policy(d, execute_policy_ftor::safe_tag{});
  (void)execute_policy(d, "unknown");
  return 0;
}
