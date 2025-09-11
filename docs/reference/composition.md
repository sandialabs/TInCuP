# Composition

Compose CPOs (or any invocables) left-to-right using `tincup::compose`:

```cpp
#include <tincup/tincup.hpp>
using namespace tincup;

// Suppose normalize(x) -> y and stringify(y) -> std::string
inline constexpr struct normalize_ftor final : cpo_base<normalize_ftor> {
  TINCUP_CPO_TAG("normalize")
  template<typename T>
    requires invocable_c<normalize_ftor, T>
    constexpr auto operator()(T&& x) const
    noexcept(nothrow_invocable_c<normalize_ftor, T>) -> invocable_t<normalize_ftor, T> {
      return tag_invoke(*this, std::forward<T>(x));
    }
  template<typename T>
    requires (!invocable_c<normalize_ftor, T>)
    constexpr void operator()(T&& x) const { this->enhanced_fail(std::forward<T>(x)); }
} normalize;

inline constexpr struct stringify_ftor final : cpo_base<stringify_ftor> {
  TINCUP_CPO_TAG("stringify")
  template<typename U>
    requires invocable_c<stringify_ftor, U>
    constexpr auto operator()(U&& u) const
    noexcept(nothrow_invocable_c<stringify_ftor, U>) -> invocable_t<stringify_ftor, U> {
      return tag_invoke(*this, std::forward<U>(u));
    }
  template<typename U>
    requires (!invocable_c<stringify_ftor, U>)
    constexpr void operator()(U&& u) const { this->enhanced_fail(std::forward<U>(u)); }
} stringify;

// Compose: stringify(normalize(x))
auto to_string = compose(normalize, stringify);
// Usage
// std::string s = to_string(obj);
```
