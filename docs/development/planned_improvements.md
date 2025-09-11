# Planned Improvements for TInCuP

This document tracks medium‑term enhancements inspired by P2279R0 "We need a language mechanism for customization points", Barry Revzin's critiques of `tag_invoke`, and comprehensive technical review feedback. These improvements aim to address fundamental customization challenges within C++20/23 constraints while maintaining the project's focus on being practical "bridge technology."

## 1. Strengthened Verification (High Priority)

**Goal**: Make CPO patterns machine-checkable and migration-ready through rigorous verification.

### Enhanced Pattern Checking
- [ ] **Concept Family Consistency**: Verify every positive `operator()` participates in the same concept family (`invocable_c`/`nothrow_invocable_c`/`invocable_t`) with identical argument substitution
- [ ] **Exact noexcept Agreement**: Ensure noexcept conditions mirror nothrow traits with exact argument lists, not just CPO name matching
- [ ] **Dead Fallback Detection**: Add pass to detect negative-case overloads that are unreachable due to overly permissive positive requires clauses
- [ ] **Tag Name Consistency**: Enforce that `TINCUP_CPO_TAG("name")` exactly matches the CPO object name and appears before any overloads

### SFINAE and Forwarding Quality
- [ ] **Perfect Forwarding Verification**: Ensure forwarding reference generation always forwards the same arity received (no stray `...` in unary cases)
- [ ] **Poison-Pill Fallbacks**: Emit curated static_assert fallbacks inside cpo_base for negative cases, ensuring users see helpful messages before ADL dumps
- [ ] **Concept-Gated Calls**: Gate all calls with concept checks first (`requires invocable_c<CPO, Args...>`) and short-circuit to curated error paths on failure

## 2. Intent Metadata & Tooling Infrastructure (High Priority) 

**Goal**: Make CPOs discoverable, documentable, and automatically migratable.

### Sidecar Metadata System
- [ ] **CPO Specification Storage**: Generate `.tincup.json` sidecar files containing full CPO specs (name, args, return contract, noexcept policy, diagnostics level)
- [ ] **Migration Mapping**: Store source location + specification data for future automated refactoring when language features arrive
- [ ] **Verifier Integration**: Extend verifier to read sidecar JSONs for comprehensive validation

### Three-Layer Concept Generation
- [ ] **Layered Concept Hierarchy**: Generate three concept layers per CPO:
  1. `*_invocable_c` (pure availability)  
  2. `*_nothrow_invocable_c` (exception contract)
  3. `*_c` (semantic requirements with return type validation)
- [ ] **Compile-Time CPO Registry**: Implement `constexpr` registry via `cpo_descriptor` and `tincup::interfaces::for_each_cpo(F&&)` hook
- [ ] **Documentation Generation**: Auto-generate Doxygen stubs and searchable HTML index from sidecar metadata

## 3. Enhanced Error Diagnostics & User Guidance

**Goal**: Transform cryptic template errors into educational, actionable guidance.

### "Why Not" Failure Analysis  
- [ ] **Common Variant Detection**: When `invocable_c<CPO, Args...>` fails, check common variants (deref, remove_const, swap_args) and emit single-line suggestions
- [ ] **Bounded Diagnostic Messages**: Keep failure explanations to one line each to avoid "diagnostic fatigue"
- [ ] **Negative Path Prioritization**: Ensure curated errors fire before walls of ADL candidates using `requires (!invocable_c<...>)` guards

### Static Analysis Integration
- [ ] **Clang-Tidy Plugin**: Implement checks for:
  - Non-hidden `tag_invoke` overloads (discourage namespace pollution)
  - Mismatched noexcept vs CPO's `_nothrow_invocable_c`
  - Basic CPO usage patterns

## 4. Conformance Testing & Validation

**Goal**: Provide opt-in verification that CPO implementations are semantically correct.

### Header-Only Testing Framework
- [ ] **Macro-Based Testing**: Implement `TINCUP_TEST(ConceptName, MyType)` macros that expand to `static_assert`s plus negative tests (wrong cv/ref, swapped args)
- [ ] **Opt-In Testing**: Use `#define TINCUP_TEST` to enable conformance test generation
- [ ] **Shape Validation**: Use `consteval` sparingly for arity and cv/ref checks, prefer `requires` for user code validation

### Implementation Guidance  
- [ ] **Canonical Examples**: Generate minimal example blocks showing correct `tag_invoke` and member-fallback versions (hidden by `#if TINCUP_EXAMPLES`)
- [ ] **Implementation Hints**: Provide guided templates for common implementation patterns

## 5. Type-Safe Composition & Pipelines (Measured Scope)

**Goal**: Enable safe CPO composition without template explosion.

### Pipeline Type Functions
- [ ] **Type Introspection**: Implement `pipeline_input_t<P>`, `pipeline_output_t<P, In>`, and `composable_c<F,G>` concepts
- [ ] **Orthogonal Design**: Keep composition above CPOs - pipelines should accept any callable, including CPOs
- [ ] **Optional Fusion**: Add `constexpr bool is_pure_c<Stage>` trait; only fuse adjacent pure, trivially-forwarding stages with `TINCUP_FUSE=ON`

### Composition Safety
- [ ] **Type Verification**: Verify `result_of<F(In)>` feeds `G` in composition chains
- [ ] **Compile-Time Cost Control**: Guard optimization features with switches to manage template instantiation cost

## 6. Language Feature Support & Future-Proofing

**Goal**: Position TInCuP for smooth migration to future language features.

### C++23+ Integration  
- [ ] **Deducing This**: Use explicit object parameters for member-fallback CPOs (C++23+, additive to C++20)
- [ ] **Module Support**: Provide `tincup.module.hpp` with exported CPOs and unexported helpers, add CI for both header-only and module builds
- [ ] **Async Patterns**: Make async CPOs a template pattern - emit both sync and `co_await`-aware overloads behind distinct tags

### Limited Parameter Support
- [ ] **Minimal NTP Support**: Add support for common non-type template parameter cases (e.g., `template<int N>` buffer sizes)
- [ ] **Template-Template Parameters**: Basic support for allocators/policies (80% of use cases, even if some diagnostics are skipped)

## 7. Developer Experience & Tooling Integration

**Goal**: Make CPO development seamless and discoverable.

### Enhanced Code Generation
- [ ] **Incremental Updates**: Add `// TINCUP-BEGIN: cpo_name` / `// TINCUP-END` guards for surgical updates without clobbering human comments
- [ ] **JSON Schema & VSCode Extension**: Ship JSON schema for spec files and VSCode extension with validation, snippets ("new CPO", "new pipeline")

### Build System Integration
- [ ] **CMake Automation**: Emit `tincup.cmake` that scans sources for `TINCUP_CPO_TAG` and auto-generates doc index target (`tincup:cpo_index`)
- [ ] **Performance Documentation**: Include compile-time and runtime micro-benchmarks (CPO call vs direct call) with published results to address performance concerns

## 8. Migration Strategy & Automated Refactoring

**Goal**: Enable zero-effort migration to future language customization features.

### Migration Infrastructure
- [ ] **Specification Preservation**: Maintain machine-readable specs alongside generated code for automated rewriting
- [ ] **Refactoring Pipeline**: Develop `spec.json → generated CPO today → auto-rewrite → customisable tomorrow` toolchain
- [ ] **Pattern Uniformity**: Leverage enforced structural consistency to enable single-pass refactoring across entire codebases

### Documentation of Migration Path
- [ ] **Migration Slides**: Create presentation materials showing concrete before/after examples of language feature migration
- [ ] **Automated Tooling**: Build proof-of-concept tools demonstrating feasible automated migration

## Implementation Priorities

Based on technical review feedback and impact analysis:

### **Phase 1 (Must Land Together - High Impact)**
- **Strengthened Verification**: Enhanced pattern checking, exact noexcept agreement, dead fallback detection
- **Sidecar Metadata System**: `.tincup.json` generation for migration readiness  
- **Three-Layer Concept Generation**: Availability + exception + semantic concepts
- **Compile-Time Registry**: `constexpr` CPO enumeration with `for_each_cpo()` hook
- **Type-Safe Pipelines**: Narrow scope composition with type function introspection only

### **Phase 2 (Developer Experience & Quality)**  
- **"Why Not" Diagnostics**: Common variant detection with single-line suggestions
- **Clang-Tidy Integration**: Basic `tag_invoke` and noexcept mismatch detection
- **Conformance Testing**: Header-only `TINCUP_TEST()` macro system
- **Incremental Code Generation**: Surgical updates with `TINCUP-BEGIN/END` guards
- **JSON Schema & VSCode Extension**: Spec validation and snippets

### **Phase 3 (Future-Proofing & Advanced Features)**
- **C++23+ Integration**: Deducing this, module support, async patterns
- **Limited Parameter Support**: Minimal NTP and template-template parameter support  
- **Build System Automation**: CMake discovery, performance benchmarking
- **Migration Tooling**: Automated refactoring pipeline demonstration

## Critical Design Constraints

### **Red Flags to Avoid** (Per Review Feedback)
- ❌ **"Runtime CPO Discovery"**: Avoid claims of enumerating user specializations - frame as compile-time registry + traits only
- ❌ **Over-Eager Optimization**: Template fusion/parallelism that explodes instantiation costs - keep switches to guard expense  
- ❌ **Verbose Diagnostics**: Ensure curated errors fire before ADL candidate walls using `requires (!invocable_c<...>)` guards first
- ❌ **Premature Complexity**: Focus on the 80% use cases for parameter support rather than comprehensive coverage

### **Success Criteria**
- **Migration Readiness**: Uniform, machine-checkable patterns enable single-pass automated refactoring to future language features
- **Measured Performance**: Compile-time cost documentation with specific percentages (e.g., "Level-3 diagnostics add ~X% to compile time")
- **Educational Value**: Transform cryptic template errors into one-line, actionable guidance
- **Bridge Technology**: Provide `tag_invoke` ergonomics today while positioning for seamless future migration

---

These improvements address the core limitations identified in P2279R0 and technical review feedback while maintaining TInCuP's role as practical "bridge technology" - delivering the ergonomics of customization points today while ensuring clean migration paths to future language features.
