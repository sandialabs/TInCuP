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

**🚀 Recommended: Use the automated pre-checkin script**

```bash
./scripts/checkin.sh  # Runs all validation automatically
```

This script automatically:
1. 📦 **Generates the single header** (`single_include/tincup.hpp`)
2. 🏷️ **Verifies copyright banners** on all source files  
3. 🔧 **Runs complete local CI** (mirrors GitHub Actions exactly)

You can run the pre-commit hook to have `git` automatically run `scripts/checkin.sh` 
before commits or pushes. 

```bash
./scripts/setup-git-hooks.sh
```


**Alternative: Individual steps**

```bash
./scripts/run_local_ci.sh         # full CI suite
./scripts/run_local_ci.sh --quick # faster subset

# Individual targets
make -f build_systems/make/Makefile test        # Python tests
make -f build_systems/make/Makefile verify-cpos # Pattern verification

# Single header maintenance
cd scripts && python generate_single_header.py  # Regenerate single header

# Copyright banner checking
python scripts/banner_check.py --fix           # Auto-fix missing banners
```

## Pull Request Checklist

- **Run pre-checkin validation**: `./scripts/checkin.sh` passes without errors
- **Single header updated**: Automatically generated (don't edit `single_include/tincup.hpp` directly)
- **Copyright banners**: Present on all new source files (auto-fixed by checkin script)
- **Tests added/updated**: For new or changed functionality (see Testing Policy)
- **Tests pass locally**: Or explain any platform-specific constraints
- **CI is green**: Or has expected, explained failures
- **Documentation updated**: If behavior or interfaces change
- **Focused changes**: Avoid unrelated refactors in the same PR

## Single Header Maintenance

The `single_include/tincup.hpp` file is **automatically generated** - never edit it directly!

- **Purpose**: Provides nlohmann::json-style single-file distribution
- **Generation**: `cd scripts && python generate_single_header.py`
- **Features**: Same API as multi-header, optimized includes, proper copyright banners
- **Testing**: `tests/test_single_header.cpp` verifies functionality
- **Automation**: The `checkin.sh` script regenerates it automatically

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
