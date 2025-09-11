# Enhanced Error Diagnostics

TInCuP addresses a major limitation identified in P2279R0: unhelpful compiler error messages when CPOs are used incorrectly. The diagnostic system detects common misuse patterns and provides clear, actionable guidance.

## Diagnostic Categories

### Pointer/Smart Pointer Misuse Detection
When you accidentally pass a pointer when a reference is expected:

```cpp
std::unique_ptr<Vector> vec_ptr = std::make_unique<Vector>();
add_assign(vec_ptr, other_vec);  // ❌ Passing smart pointer instead of object
```

Enhanced message:
```
CPO: No valid tag_invoke overload for these argument types, but there IS a valid 
overload for the dereferenced arguments. Some arguments appear to be pointers/smart_ptrs 
that may need explicit dereferencing. Consider: add_assign(*vec_ptr, other_vec)
```

### Const-Qualification Misuse Detection
When you pass const objects to mutating operations:

```cpp
const Vector vec{1.0, 2.0};
add_assign(vec, other_vec);  // ❌ Passing const object to mutating operation
```

Enhanced message:
```
CPO: No valid tag_invoke overload for these argument types, but there IS a valid 
overload for non-const arguments. You may be passing const objects to a mutating operation. 
Consider: add_assign(non_const_vec, other_vec)
```

### Combined Issue Detection
When multiple corrections are needed:

```cpp
const std::unique_ptr<Vector> vec_ptr = std::make_unique<Vector>();
add_assign(vec_ptr, other_vec);  // ❌ Both const and pointer issues
```

Enhanced message:
```
CPO: No valid tag_invoke overload for these argument types, but there IS a valid 
overload for dereferenced non-const arguments. You may need both pointer dereferencing 
and to remove const-qualification.
```

### Argument Order Detection
When binary arguments are swapped:

```cpp
transform(source, target, func);  // ❌ Arguments in wrong order
```

Enhanced message:
```
CPO: No valid tag_invoke overload for these argument types, but there IS a valid 
overload with reordered arguments. You may have swapped argument positions. 
Check the expected argument order for this CPO.
```

### Wrong Argument Count Detection
When the number of arguments is incorrect:

```cpp
binary_op(single_arg);           // ❌ Missing second argument  
binary_op(arg1, arg2, extra);    // ❌ Too many arguments
```

Enhanced message:
```
CPO: No valid tag_invoke overload for this number of arguments, but there IS a valid 
overload with different arity. You may be passing too many or too few arguments. 
Check the expected number of arguments for this CPO.
```

## C++23/C++26 Enhancements

With C++23, TInCuP provides more sophisticated error analysis with detailed template instantiation context.

With C++26 (GCC 14+/Clang 19+, `-std=c++2c`), user-generated static_assert messages (P2741R3) include the specific CPO name in diagnostics.

## Performance Controls

For performance-critical builds, TInCuP provides fine-grained control over diagnostic overhead:

```cpp
// Disable specific diagnostic categories
#define TINCUP_DISABLE_POINTER_DIAGNOSTICS
#define TINCUP_DISABLE_CONST_DIAGNOSTICS
#define TINCUP_DISABLE_ORDER_DIAGNOSTICS
#define TINCUP_DISABLE_ARITY_DIAGNOSTICS

// Or disable all enhanced diagnostics at once
#define TINCUP_DISABLE_ALL_DIAGNOSTICS
#define TINCUP_MINIMAL_DIAGNOSTICS  // alias

// Or use single-knob diagnostic level control
#define TINCUP_DIAGNOSTIC_LEVEL 0  // all off
#define TINCUP_DIAGNOSTIC_LEVEL 1  // keep pointer/const
#define TINCUP_DIAGNOSTIC_LEVEL 2  // keep pointer/const/order
#define TINCUP_DIAGNOSTIC_LEVEL 3  // all on (default)
```

Compilation impact:
- Default: bounded extra checks per failed call
- Minimal mode: fastest compilations
- Selective: choose which categories matter

## Technical Benefits

- Educational: teaches correct usage patterns
- Version-aware: C++23/C++26 improvements where available
- Zero runtime cost: diagnostics are compile-time only
- Comprehensive coverage: five major misuse categories
- Performance conscious: opt-out controls

## References

- Customization Point Design in C++11 and Beyond
- Suggested Design for Customization Points
- P1665r0 Tag based customization points
- P1895R0 tag_invoke: A general pattern for supporting customisable functions
- P2279R0 We need a language mechanism for customization points
- P2547R0 Language support for customisable functions
- P2855R1 Member customization points for Senders and Receivers

