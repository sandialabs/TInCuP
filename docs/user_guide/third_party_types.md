# Third-Party Types

When you need to customize CPOs for types you do not control (for example, `Kokkos::View<...>>`), TInCuP provides a formatter-style extension point that avoids adding symbols to third-party namespaces and requires no wrappers at call sites.

- Extension point: specialize `tincup::cpo_impl<CPO, T>` in your project and implement `static auto call(T&, Args&&...)`.
- Discovery: This trait mirrors the approach used by `std::formatter`. Specializations live in namespace `tincup` and do not require modifying third-party headers.
- Generation support: use the generator to emit a skeleton specialization.

Trait impl targets and generics

- Use '$Name' in --impl-target to declare a template parameter 'Name'.
- Use '$Name...' to declare a parameter pack.
- Use '...' to declare an anonymous pack (maps to 'typename... P').
- Bare identifiers (e.g., 'double') are treated as concrete types and will not create template parameters.
- Named packs without '$' (e.g., 'Rest...') are invalid and will error — write '$Rest...' instead.

Examples:

```bash
# Single generic parameter
cpo-generator '{"cpo_name":"foo","args":["$V&: x"]}' \
  --impl-target 'MyContainer<$T>' --trait-impl-only

# Head + tail pack (e.g., Kokkos::View first parameter + rest)
cpo-generator '{"cpo_name":"add_in_place","args":["$V&&: y","$const V&: x"]}' \
  --impl-target 'Kokkos::View<$T, $Rest...>' --trait-impl-only

# Concrete first parameter, generic tail pack
cpo-generator '{"cpo_name":"add_in_place","args":["$V&&: y","$const V&: x"]}' \
  --impl-target 'Kokkos::View<double, $Rest...>' --trait-impl-only
```

The corresponding specialization headers use 'typename' consistently, for example:

```cpp
namespace tincup {
template<typename T, typename... Rest>
struct cpo_impl<add_in_place_ftor, Kokkos::View<T, Rest...>> {
  template<typename... Args>
  static auto call(Kokkos::View<T, Rest...>& target, Args&&... args) -> /* auto */;
};
} // namespace tincup
```

Example: generate a trait specialization for a string-dispatch CPO targeting a third-party template type

```bash
cpo-generator '{"cpo_name":"execute_policy", "args":["$T&: data"], "runtime_dispatch":{"type":"string","dispatch_arg":"policy","options":["fast","safe","debug"]}}'   --emit-trait-impl --impl-target 'Kokkos::View<...>' --out include/myproj/execute_policy_impl.hpp
```

The generated skeleton looks like this (simplified):

```cpp
namespace tincup {
template<class... P>
struct cpo_impl<execute_policy_ftor, Kokkos::View<P...>> {
  template<class... Args>
  static auto call(Kokkos::View<P...>& view, Args&&... args)
      -> /* return type */ {
    // TODO: implement using 'view' and (args...)
  }
};
} // namespace tincup
```

Notes:
- You can wrap emission in a macro guard with `--impl-guard MACRO`.
- This approach avoids defining functions in third-party namespaces and does not require inheritance or wrappers at call sites.
- It is suitable for both concrete and templated third-party types; use `'...'` in `--impl-target` to denote a template parameter pack (e.g., `Kokkos::View<...>`).

## Trait Detection Helpers and Safe Patterns

TInCuP provides opt-in helpers to detect whether a `tincup::cpo_impl` specialization exists for a given call. Use these to write “trait-first, generic-fallback” ADL shims without ambiguity.

- `tincup::has_cpo_impl_for_c<CPO, Target, Args...>`: true if `cpo_impl<CPO, Target>::call(Target&, Args...)` is well-formed.
- `tincup::has_specialized_cpo_impl_c<CPO, Args...>`: detects a specialization based on the principal argument (first parameter) of the call using `cpo_traits<CPO, Args...>::arg_t<0>`.
- `_v` variable templates and `_t` aliases are also available.

Recommended pattern (mutually exclusive overloads):

```cpp
namespace myproj {
// Generic fallback only when no trait specialization exists
template<std::ranges::range R>
  requires (!tincup::has_cpo_impl_for_c<add_in_place_ftor, R, R&, const R&>)
void tag_invoke(add_in_place_ftor, R& y, const R& x) {
  std::ranges::transform(y, x, std::ranges::begin(y), std::plus<>{});
}

// Forwarding shim only when a trait specialization exists
template<typename T, typename Alloc>
  requires (tincup::has_cpo_impl_for_c<add_in_place_ftor,
                                       std::vector<T,Alloc>, 
                                       std::vector<T,Alloc>&,
                                       const std::vector<T,Alloc>&>)
void tag_invoke(add_in_place_ftor, std::vector<T,Alloc>& y,
                const std::vector<T,Alloc>& x) {
  tincup::cpo_impl<add_in_place_ftor, std::vector<T,Alloc>>::call(y, x);
}
} // namespace myproj
```

Guidance and cautions:
- Make overloads disjoint with explicit `requires` so both cannot be viable. Avoid ambiguous overloads.
- Principal argument policy: detection keys off the first parameter; if your principal type differs, write a custom detector.
- When constraining `tag_invoke` overloads for the same CPO, prefer `has_cpo_impl_for_c` with an explicit target type. Using `has_specialized_cpo_impl_c` there can recursively trigger CPO introspection and cause constraint recursion.
- Do not place shims in foreign namespaces (`std`, third-party). Keep ADL-visible shims in your CPO’s namespace.
- Keep trait specializations in one place to avoid ODR violations.
- Performance varies by backend; benchmark both paths.

### Trait + ADL Shim (std::vector)

Use an ADL-visible shim in your CPO's namespace that forwards to the `tincup::cpo_impl` specialization. This keeps the core CPO free of TPL references and requires no wrappers.

- Command (emit trait + shim):

```bash
cpo-generator --from-registry add_in_place_ftor \
  --impl-target 'std::vector<$T, $Alloc>' \
  --emit-trait-impl --emit-adl-shim --shim-namespace 'myproj' \
  --out include/myproj/add_in_place_vector.hpp
```

- Result (simplified):

```cpp
// myproj/add_in_place.hpp (core CPO)
namespace myproj {
inline constexpr struct add_in_place_ftor final : tincup::cpo_base<add_in_place_ftor> {
  TINCUP_CPO_TAG("add_in_place");
} add_in_place;
} // namespace myproj

// Integration header (trait + ADL shim)
namespace tincup {
template<typename T, typename Alloc>
struct cpo_impl<myproj::add_in_place_ftor, std::vector<T, Alloc>> {
  static void call(std::vector<T, Alloc>& y, const std::vector<T, Alloc>& x);
};
} // namespace tincup

namespace myproj {
template<typename T, typename Alloc, typename... Args>
constexpr auto tag_invoke(add_in_place_ftor, std::vector<T, Alloc>& y, Args&&... args)
  noexcept(noexcept(tincup::cpo_impl<add_in_place_ftor, std::vector<T, Alloc>>::call(y, std::forward<Args>(args)...)))
  -> decltype(tincup::cpo_impl<add_in_place_ftor, std::vector<T, Alloc>>::call(y, std::forward<Args>(args)...));
} // namespace myproj
```

- Behavior:
  - Calls like `myproj::add_in_place(vec, other)` resolve to the shim via ADL, which forwards to `cpo_impl<...>::call(...)`.
  - No third-party symbols appear in the core CPO definition.

```