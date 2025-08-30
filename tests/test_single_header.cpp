/**
TInCuP - A library for generating and validating C++ customization point objects that use `tag_invoke`

Copyright (c) National Technology & Engineering Solutions of Sandia, 
LLC (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S. 
Government retains certain rights in this software.

Questions? Contact Greg von Winckel (gvonwin@sandia.gov)
*/

// Test that verifies the single header version works correctly
#include "../single_include/tincup.hpp"
#include <string>
#include <cassert>
#include <iostream>

namespace test_single_header {

struct Data {};

// Simple CPO to test the single header
inline constexpr struct simple_test_ftor final : tincup::cpo_base<simple_test_ftor> {
  TINCUP_CPO_TAG("simple_test")

  template<typename T>
  constexpr auto operator()(T& input) const {
    return tag_invoke(*this, input);
  }
} simple_test{};

// Implementation for Data
constexpr auto tag_invoke(simple_test_ftor, Data&) {
  return 42;
}

// Implementation for string
inline auto tag_invoke(simple_test_ftor, std::string& s) {
  return s.length();
}

// Test boolean dispatch
inline constexpr struct bool_test_ftor final : tincup::cpo_base<bool_test_ftor> {
  TINCUP_CPO_TAG("bool_test")

  static constexpr struct true_tag {} true_path{};
  static constexpr struct false_tag {} false_path{};

  template<typename T>
  constexpr auto operator()(T& input, bool selector) const {
    tincup::BoolDispatch dispatcher(selector);
    return dispatcher.receive([&](auto dispatch_constant) {
      if constexpr (dispatch_constant.value) {
        return tag_invoke(*this, input, true_path);
      } else {
        return tag_invoke(*this, input, false_path);
      }
    });
  }
} bool_test{};

// Implementations for bool test
constexpr auto tag_invoke(bool_test_ftor, Data&, bool_test_ftor::true_tag) {
  return "true";
}

constexpr auto tag_invoke(bool_test_ftor, Data&, bool_test_ftor::false_tag) {
  return "false";
}

} // namespace test_single_header

int main() {
  using namespace test_single_header;
  
  std::cout << "Testing single header version of TInCuP..." << std::endl;
  
  // Test 1: Basic CPO functionality
  Data data;
  auto result1 = simple_test(data);
  assert(result1 == 42);
  std::cout << "✓ Basic CPO test passed: " << result1 << std::endl;
  
  // Test 2: String test
  std::string test_str = "hello";
  auto result2 = simple_test(test_str);
  assert(result2 == 5);
  std::cout << "✓ String CPO test passed: " << result2 << std::endl;
  
  // Test 3: Boolean dispatch
  auto result3 = bool_test(data, true);
  assert(std::string(result3) == "true");
  std::cout << "✓ Boolean dispatch (true) test passed: " << result3 << std::endl;
  
  auto result4 = bool_test(data, false);
  assert(std::string(result4) == "false");
  std::cout << "✓ Boolean dispatch (false) test passed: " << result4 << std::endl;
  
  // Test 4: Type traits
  static_assert(tincup::is_invocable_v<simple_test_ftor, Data&>);
  static_assert(tincup::is_invocable_v<simple_test_ftor, std::string&>);
  // Note: bool_test_ftor needs Data&, bool arguments which might not match is_invocable_v check
  std::cout << "✓ Type traits tests passed" << std::endl;
  
  std::cout << "All single header tests passed! ✓" << std::endl;
  return 0;
}