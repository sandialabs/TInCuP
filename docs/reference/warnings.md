# Documentation-Related Compiler Warnings: Causes and Remedies

This note summarizes common documentation warnings seen when compiling with Clang
(`-Wdocumentation`, `-Wdocumentation-unknown-command`, `-Wpedantic`) and practical
ways you can address them in your project. TInCuP does not enforce any one approach.

## Clang: Unknown Doxygen Commands

- Problem: Clang has a hard-coded set of recognized doc commands. Custom tags like
  `@cpo_impl`, `@cpo_adl_shim`, `@cpo_example`, etc., will trigger
  `-Wdocumentation-unknown-command` even if you define aliases in your Doxyfile.
- Applies to: Clang/LLVM. GCC/MSVC generally do not warn here.

Remedies you can choose from:
- Use standard tags: Prefer `@brief`, `@param`, `@tparam`, `@note`, `@par`, `@section`.
- Move custom tags to sidecar docs: Put `\defgroup`/`\ingroup` and custom-tag content
  in `.dox`/`.md` files that are not compiled by the compiler (only read by Doxygen).
- Preprocessor guard for Doxygen-only blocks: Wrap custom-tag blocks in
  `#if defined(DOXYGEN) ... #endif` and add `PREDEFINED += DOXYGEN` in your Doxyfile
  so Doxygen still sees them while Clang does not.
- Suppress via compiler flags: Add `-Wno-documentation-unknown-command` or
  `-Wno-documentation` where appropriate (target- or project-wide).
- Local suppression: Surround the custom-tag block with
  `#pragma clang diagnostic push` / `#pragma clang diagnostic ignored "-Wdocumentation-unknown-command"`
  … `#pragma clang diagnostic pop`.

## Grouping and Aliases

- Doxygen aliases (Doxyfile `ALIASES += ...`) only affect Doxygen output and
  do not influence compiler diagnostics.
- `\defgroup`/`\ingroup` may also be unknown to Clang and can trigger warnings if
  present in headers. Prefer placing group definitions in `.dox` files.

## Practical Patterns

- Doxygen-only sections (recommended when strict Clang warnings are enabled):
  - Doxyfile: `PREDEFINED += DOXYGEN`
  - Headers:
    ```cpp
    #if defined(DOXYGEN)
    /**
     * @cpo_impl
     * @ingroup tincup_cpo_integrations
     */
    #endif
    ```
- Compiler-quiet comments (no custom tags):
  - Encode taxonomy with standard tags, e.g. `@par CPO Integration:` and
    `@note Forwarded via ADL shim to trait specialization`.
- Local suppression (surgical):
  ```cpp
  #if defined(__clang__)
  #  pragma clang diagnostic push
  #  pragma clang diagnostic ignored "-Wdocumentation-unknown-command"
  #endif
  /** @cpo_adl_shim @ingroup tincup_cpo_integrations */
  #if defined(__clang__)
  #  pragma clang diagnostic pop
  #endif
  ```

## Meson/CMake Snippets (optional)

- Meson (disable unknown command warnings project-wide):
  ```meson
  add_project_arguments('-Wno-documentation-unknown-command', language: 'cpp')
  # or per target: executable('x', srcs, cpp_args: ['-Wno-documentation-unknown-command'])
  ```
- CMake (per target):
  ```cmake
  target_compile_options(my_target PRIVATE -Wno-documentation-unknown-command)
  # or broader: -Wno-documentation
  ```

Pick the remedy that best fits your warning policy and documentation style.

