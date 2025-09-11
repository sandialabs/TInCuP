# CPO Introspection

TInCuP provides comprehensive introspection capabilities for CPOs through `cpo_traits`:

```cpp
#include <tincup/tincup.hpp>
using namespace tincup;

// Example CPO for introspection
struct my_cpo_ftor final : cpo_base<my_cpo_ftor> {
    template<typename T, typename U>
        requires invocable_c<my_cpo_ftor, T&, const U&>
    constexpr auto operator()(T& target, const U& source) const
        noexcept(nothrow_invocable_c<my_cpo_ftor, T&, const U&>)
        -> invocable_t<my_cpo_ftor, T&, const U&> {
        return tag_invoke(*this, target, source);
    }
    
    template<typename T, typename U>
        requires (!invocable_c<my_cpo_ftor, T&, const U&>)
    constexpr void operator()(T& target, const U& source) const {
        this->enhanced_fail(target, source);
    }
} my_cpo;

// Introspect the CPO
using traits = cpo_traits<my_cpo_ftor, std::string&, const int&>;

static_assert(traits::arity == 2);                    // Number of arguments
static_assert(traits::invocable);                     // Can be invoked with these types
static_assert(traits::nothrow_invocable);             // Is noexcept with these types
static_assert(traits::is_void_returning);             // Returns void
static_assert(traits::all_args_are_refs);             // All arguments are references
static_assert(!traits::all_args_are_const_refs);      // Not all are const references

// Access individual argument types
using first_arg = traits::arg_t<0>;   // std::string&
using second_arg = traits::arg_t<1>;  // const int&

// Get a hint about the signature for debugging
constexpr auto sig = traits::signature_hint();  // Returns "(T, U)"
```

### Available Introspection Features

- **`invocable`**: Whether the CPO can be invoked with given argument types
- **`nothrow_invocable`**: Whether the CPO is noexcept with given argument types  
- **`arity`**: Number of arguments
- **`return_t`**: Return type (void if not invocable)
- **`is_void_returning`**: Whether the return type is void
- **`is_const_invocable`**: Whether the CPO itself is const-qualified
- **`all_args_are_refs`**: Whether all arguments are reference types
- **`all_args_are_const_refs`**: Whether all arguments are const references
- **`arg_t<I>`**: Type of the I-th argument
- **`signature_hint()`**: String representation for debugging
