# Testing Policy

This project requires tests to accompany major new functionality. Bug fixes should include a regression test when practical.

## Policy

- New features: Add unit and/or integration tests that exercise the new behavior.
- Regressions: Add a test that fails before the fix and passes after.
- Interfaces: Update or add tests when public behavior changes.
- Platforms: Prefer deterministic, cross‑platform tests; keep platform‑specific logic behind feature flags or separate jobs.

## Scope and Locations

- Python tooling tests: `tests/*.py` (e.g., generator, verification)
- C++ library tests: `tests/*.cpp` and build system smoke tests
- Editor/IDE integration tests: `tests/*_integration_test.sh`

## CI Enforcement

Automated tests run in CI across multiple Python versions and toolchains (GCC/Clang/MSVC; CMake/Meson). PRs adding significant functionality without tests will not be merged absent a clear, documented justification.

## Fuzzing

We run a lightweight fuzzing job (Python, atheris/libFuzzer) against the generator pipeline to uncover unexpected exceptions on malformed inputs. See `tests/fuzz/fuzz_cpo_generator.py`. The job is time-bounded and non-blocking; crashes will be investigated and converted into regression tests where practical.

## Evidence of Adherence

Recent changes (e.g., CPO generator improvements, verification enhancements, Windows/MSYS2 and Meson/CMake smoke tests) include accompanying tests under `tests/` and dedicated CI jobs that exercise them.
