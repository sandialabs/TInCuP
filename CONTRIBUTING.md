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

- Tests pass locally (or explain any platform-specific constraints)
- CI is green (or has expected, explained failures)
- Update docs if behavior or interfaces change
- Keep changes focused; avoid unrelated refactors in the same PR

## Code of Conduct

Please review and follow our [Code of Conduct](CODE_OF_CONDUCT.md).

## Security

If you believe you’ve found a security vulnerability, please follow our [Security Policy](SECURITY.md) and do not open a public issue with sensitive details.

