# Makefile for tincup project
# Provides convenient targets for development workflow

.PHONY: test test-patterns test-verification test-coverage install-dev clean help ci-local ci-quick test-cmake test-meson test-examples test-headers update-readme-examples preview-readme-examples

# Default target
help:
	@echo "Available targets:"
	@echo ""
	@echo "=== Testing ==="
	@echo "  test              - Run all Python tests"
	@echo "  ci-local          - Run complete local CI suite (mirrors GitHub Actions)"
	@echo "  ci-quick          - Run quick local CI suite (faster subset)"
	@echo ""  
	@echo "=== Specific Tests ==="
	@echo "  test-python       - Run Python test suite only"
	@echo "  test-cmake        - Test CMake build only"
	@echo "  test-meson        - Test Meson build only"
	@echo "  test-examples     - Test example compilation only"
	@echo "  test-headers      - Test header-only compilation"
	@echo ""
	@echo "=== Development ==="
	@echo "  test-patterns     - Run pattern generation tests only"
	@echo "  test-verification - Run verification tests only" 
	@echo "  test-coverage     - Run tests with coverage reporting"
	@echo "  install-dev       - Install development dependencies"
	@echo "  verify-cpos       - Verify CPO patterns in project"
	@echo "  generate-cpo-registry - Scan headers and write docs/cpo_registry.{json,md}"
	@echo "  update-readme-examples - Update README.md examples with current generator output"
	@echo "  preview-readme-examples - Preview README.md examples updates without applying"
	@echo "  clean             - Clean up generated files"

# Run all tests
test:
	python3 tests/run_tests.py --verbose

# Run specific test categories
test-patterns:
	python3 tests/run_tests.py --pattern --verbose

test-verification:
	python3 tests/run_tests.py --verification --verbose

# Run tests with coverage
test-coverage:
	python3 tests/run_tests.py --coverage

# Install development dependencies
install-dev:
	pip install -e .
	pip install -r tests/requirements.txt

# Verify CPO patterns in the project
verify-cpos:
	python3 -m cpo_tools.cpo_verification .

# Generate a registry of all CPO tags
generate-cpo-registry:
	python3 cpo_tools/cpo_registry.py --root include --out docs

# Update README.md examples with current generator output (JSON-driven)
update-readme-examples:
	python3 scripts/update_readme_examples.py

# Preview README.md examples updates without applying
preview-readme-examples:
	python3 scripts/update_readme_examples.py --dry-run

# Git hook for pre-commit CPO verification
install-git-hook:
	@echo "#!/bin/bash" > .git/hooks/pre-commit
	@echo "python3 -m cpo_tools.cpo_verification . --strict" >> .git/hooks/pre-commit
	@chmod +x .git/hooks/pre-commit
	@echo "Git pre-commit hook installed"

# Local CI testing (mirrors GitHub Actions exactly)
ci-local:
	bash scripts/run_local_ci.sh

# Quick local testing (faster subset)
ci-quick:
	bash scripts/run_local_ci.sh --quick

# Individual test categories
test-python: install-dev
	python3 tests/run_tests.py --verbose

test-cmake:
	@echo "Testing CMake build..."
	@BUILD_DIR="local_test_cmake" && \
	rm -rf $$BUILD_DIR && \
	cmake -S . -B $$BUILD_DIR && \
	cmake --build $$BUILD_DIR && \
	echo "✓ CMake build test passed"

test-meson:
	@echo "Testing Meson build..."
	@BUILD_DIR="local_test_meson" && \
	rm -rf $$BUILD_DIR && \
	meson setup $$BUILD_DIR build_systems/meson -Denable_examples=true && \
	meson compile -C $$BUILD_DIR && \
	echo "✓ Meson build test passed"

test-examples:
	@echo "Testing examples compilation..."
	@g++ -std=c++20 -I include examples/serialize/test_serialization.cpp -o test_serialize && \
	./test_serialize && \
	rm test_serialize && \
	echo "✓ Example tests passed"

test-headers:
	@echo "Testing header-only compilation..."
	@echo "#include <tincup/tincup.hpp>" > header_test.cpp && \
	echo "int main() { return 0; }" >> header_test.cpp && \
	g++ -std=c++20 -I include header_test.cpp -o header_test && \
	./header_test && \
	rm header_test header_test.cpp && \
	echo "✓ Header-only test passed"

# Clean up
clean:
	find . -type f -name "*.pyc" -delete
	find . -type d -name "__pycache__" -delete
	rm -f .coverage
	rm -rf local_ci_build local_test_cmake local_test_meson
