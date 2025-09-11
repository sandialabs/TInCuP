# TInCuP Networking Serialization Example Makefile

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -I../..

# Targets
SIMPLE_TARGET = simple_example
FULL_TARGET = networking_example
SIMPLE_SOURCE = simple_main.cpp
FULL_SOURCE = main.cpp

# Default target - build the working simple example
all: $(SIMPLE_TARGET)

# Build the simple, working example
simple: $(SIMPLE_TARGET)

$(SIMPLE_TARGET): $(SIMPLE_SOURCE)
	$(CXX) $(CXXFLAGS) $(SIMPLE_SOURCE) -o $(SIMPLE_TARGET)

# Build the comprehensive example (has compilation issues but shows full features)
full: $(FULL_TARGET)

$(FULL_TARGET): $(FULL_SOURCE)
	$(CXX) $(CXXFLAGS) $(FULL_SOURCE) -o $(FULL_TARGET)

# Run the simple example
run: $(SIMPLE_TARGET)
	./$(SIMPLE_TARGET)

# Run the full example (if it compiles)
run-full: $(FULL_TARGET)
	./$(FULL_TARGET)

# Test with debug info
debug: CXXFLAGS += -g -DDEBUG
debug: $(SIMPLE_TARGET)

# Clean build artifacts  
clean:
	rm -f $(SIMPLE_TARGET) $(FULL_TARGET)

# Help target
help:
	@echo "TInCuP Networking Serialization Example"
	@echo "========================================"
	@echo ""
	@echo "Available targets:"
	@echo "  simple     - Build the simple, working example (default)"
	@echo "  full       - Build the comprehensive example (may have compilation issues)"
	@echo "  run        - Build and run the simple example"
	@echo "  run-full   - Build and run the full example"
	@echo "  debug      - Build with debug information"  
	@echo "  clean      - Remove build artifacts"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make           # Build and shows working TInCuP serialization"
	@echo "  make run       # Run the simple example"
	@echo "  make full      # Try to build comprehensive example"
	@echo ""
	@echo "The simple example demonstrates:"
	@echo "  • Fundamental type serialization (int, float, string)"
	@echo "  • STL container serialization (vector)"
	@echo "  • Round-trip serialization/deserialization"
	@echo "  • TInCuP CPO interface"

.PHONY: all simple full run run-full debug clean help