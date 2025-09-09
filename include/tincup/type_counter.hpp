#pragma once

#include "type_list.hpp"

namespace tincup {

template<typename... Unique>
struct type_counter {

  template<typename... Remaining>
  static constexpr std::size_t count() {
    if constexpr (sizeof...(Remaining) == 0) {
      return sizeof...(Unique);
    } else {
      return process_first<Remaining...>();
    }
  }
  
private:
  template<typename First, typename... Rest>
  static constexpr std::size_t process_first() {
    using clean_first = std::remove_cvref_t<First>;
    
    if constexpr (tincup::type_list<Unique...>::template contains_type<clean_first>) {
      return type_counter<Unique...>::template count<Rest...>();
    } else {
      return type_counter<Unique..., clean_first>::template count<Rest...>();
    }
  }
};

template<typename... Args>
constexpr std::size_t count_unique_types() {
  return type_counter<>::template count<std::remove_cvref_t<Args>...>();
}

} // namespace tincup
