# TInCuP - Documentation and Examples Makefile
# 
# This Makefile provides convenient targets for documentation generation
# and example management. For C++ builds, use CMake as usual.

# Python command (try python3, fall back to python)
PYTHON := $(shell command -v python3 2> /dev/null || echo python)

# Documentation targets
.PHONY: docs examples single-header help clean-docs examples-help

# Default target
help:
	@echo "TInCuP - Documentation and Examples"
	@echo "====================================="
	@echo ""
	@echo "Documentation targets:"
	@echo "  examples        - Regenerate docs/examples.md from JSON config"
	@echo "  single-header   - Regenerate single_include/tincup.hpp"
	@echo "  examples-help   - Show help for individual example projects"
	@echo "  clean-docs      - Clean generated documentation files"
	@echo "  help           - Show this help message"
	@echo ""
	@echo "Example projects (self-contained with own Makefiles):"
	@echo "  examples/serialize/    - Basic CPO serialization example"
	@echo "  examples/networking/   - Production-ready networking serialization"
	@echo ""
	@echo "Quick start:"
	@echo "  make examples          # Regenerate examples documentation"
	@echo "  cd examples/serialize && make run   # Try basic example"
	@echo "  cd examples/networking && make run  # Try advanced example"
	@echo ""
	@echo "For C++ library builds, use CMake:"
	@echo "  mkdir build && cd build"
	@echo "  cmake .. && make"

# Regenerate examples documentation from JSON
examples:
	@echo "Regenerating examples documentation..."
	$(PYTHON) scripts/generate_examples_doc.py
	@echo "Examples documentation updated in docs/examples.md"

# Regenerate single header
single-header:
	@echo "Regenerating single header..."
	$(PYTHON) scripts/generate_single_header.py
	@echo "Single header updated in single_include/tincup.hpp"

# Show help for example projects
examples-help:
	@echo "Example Projects Help"
	@echo "===================="
	@echo ""
	@echo "Serialize Example (Basic):"
	@echo "  cd examples/serialize"
	@echo "  make help              # Show available targets"
	@echo "  make run               # Build and run example"
	@echo ""
	@echo "Networking Example (Advanced):"
	@echo "  cd examples/networking"
	@echo "  make help              # Show available targets" 
	@echo "  make run               # Build and run simple example"
	@echo "  make full              # Try to build comprehensive example"
	@echo ""
	@echo "Both examples are self-contained with their own documentation."

# Clean generated documentation
clean-docs:
	@echo "Cleaning generated documentation..."
	@rm -f docs/examples.md
	@echo "Generated documentation files removed"

# Test that Python tools are available
test-python:
	@echo "Testing Python environment..."
	@$(PYTHON) --version
	@$(PYTHON) -c "import json, subprocess, pathlib; print('Required modules available')"
	@echo "Python environment OK"