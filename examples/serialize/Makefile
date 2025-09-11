# TInCuP Serialize Example Makefile

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -I../..

# Targets
TARGET = test_serialization
SOURCE = test_serialization.cpp
DEBUG_TARGET = debug_test
DEBUG_SOURCE = debug_test.cpp

# Default target - build the main example
all: $(TARGET)

# Build the main serialization example
$(TARGET): $(SOURCE) serialize.hpp
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $(TARGET)

# Build the debug test (if it exists)
debug-test: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_SOURCE) serialize.hpp
	$(CXX) $(CXXFLAGS) $(DEBUG_SOURCE) -o $(DEBUG_TARGET)

# Run the main example
run: $(TARGET)
	./$(TARGET)

# Run debug test
run-debug: $(DEBUG_TARGET)
	./$(DEBUG_TARGET)

# Test with debug info
debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET) $(DEBUG_TARGET)

# Test compilation with different C++ standards
test-cpp20:
	$(CXX) -std=c++20 $(CXXFLAGS) $(SOURCE) -o $(TARGET)_cpp20

test-cpp23:
	$(CXX) -std=c++23 $(CXXFLAGS) $(SOURCE) -o $(TARGET)_cpp23

# Help target
help:
	@echo "TInCuP Serialize Example"
	@echo "========================"
	@echo ""
	@echo "This example demonstrates basic TInCuP serialization using CPOs"
	@echo "and the tag_invoke protocol for JSON string generation."
	@echo ""
	@echo "Available targets:"
	@echo "  all         - Build the main serialization example (default)"
	@echo "  debug-test  - Build the debug test example"
	@echo "  run         - Build and run the main example"
	@echo "  run-debug   - Build and run the debug test"
	@echo "  debug       - Build with debug information"
	@echo "  clean       - Remove build artifacts"
	@echo "  test-cpp20  - Test C++20 compilation"
	@echo "  test-cpp23  - Test C++23 compilation"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Examples:"
	@echo "  make           # Build the serialization example"
	@echo "  make run       # Build and run the example"
	@echo "  make clean     # Clean up build files"
	@echo ""
	@echo "The example demonstrates:"
	@echo "  • Basic CPO implementation with tag_invoke"
	@echo "  • User-defined type serialization (Person struct)"
	@echo "  • Third-party type serialization (std::vector via cpo_impl)"
	@echo "  • JSON string output format"

.PHONY: all debug-test run run-debug debug clean test-cpp20 test-cpp23 help