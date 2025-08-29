# Contributing to TInCuP

Thanks for your interest in contributing! This guide will help you get set up.

## Development Setup

1. Clone the repository and create a branch.
2. Install Python tooling (for the generator and tests):
   
   ```bash
   pip install -e .
   ```

3. (Optional) Install dev tools:
   
   ```bash
   make -f build_systems/make/Makefile install-dev
   ```

## Validate Changes Locally

Run the local CI mirror before pushing:

```bash
./run_local_ci.sh         # full suite
./run_local_ci.sh --quick # faster subset
```

Or individual targets:

```bash
make -f build_systems/make/Makefile test        # Python tests
make -f build_systems/make/Makefile verify-cpos # Pattern verification
```

## Pull Request Checklist

- Tests are added/updated for new or changed functionality (see Testing Policy)
- Tests pass locally (or explain any platform-specific constraints)
- CI is green (or has expected, explained failures)
- Update docs if behavior or interfaces change
- Keep changes focused; avoid unrelated refactors in the same PR

## Testing Policy

As major new functionality is added, corresponding automated tests MUST be added to the test suite. Bug fixes SHOULD include a regression test where practical.

- Scope: unit tests for pure logic; integration tests for build systems and editor tooling; smoke tests for cross‑platform toolchains.
- Location: Python tests live under `tests/*.py`; C++ smoke/unit tests live under `tests/*.cpp` or the build system test folders.
- Enforcement: PRs that add significant functionality without tests will not be merged absent a clear, documented justification.
- Evidence: Recent major changes (e.g., generator/verification updates, MSVC support, Meson/CMake smoke tests) include accompanying tests in `tests/` and CI jobs exercising them.

See also: [docs/TESTING_POLICY.md](docs/TESTING_POLICY.md)

## Code of Conduct

Please review and follow our [Code of Conduct](CODE_OF_CONDUCT.md).

## Security

If you believe you’ve found a security vulnerability, please follow our [Security Policy](SECURITY.md) and do not open a public issue with sensitive details.

## Branching & Release Workflow

- Branching model (GitFlow‑lite):
  - Feature/bug branches off `develop` (e.g., `feature/...`, `fix/...`).
  - Open PRs into `develop` for review and CI.
  - Periodically merge `develop` into `main` to release.
  - Tag releases on `main` (SemVer). PyPI publishes from `main` only.
- Hotfixes:
  - Branch off `main` (e.g., `hotfix/...`), PR into `main`, tag, then back‑merge into `develop`.
- CI:
  - CI runs on pushes to `main` and `develop`, and on PRs targeting either branch.
  - Warnings are treated as errors for our tests/examples; consumers aren’t affected.
- Protections (recommended):
  - Enable branch protection for `main` and `develop` (require PR reviews and passing checks; disallow force‑push).
