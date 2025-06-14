# JStream Benchmark Makefile
# Usage:
#   make build    - Build the benchmark project
#   make run      - Run the benchmark with default iterations (100000)
#   make run-fast - Run the benchmark with fewer iterations (10000) for quick testing
#   make clean    - Clean build artifacts

.PHONY: build run run-fast clean

# Default target
all: build

# Build the benchmark project
build:
	@echo "Building JStream Benchmark..."
	dotnet build --configuration Release

# Run benchmark with default iterations
run: build
	@echo "Running JStream Benchmark (100000 iterations)..."
	dotnet run --configuration Release --no-build

# Run benchmark with fewer iterations for testing
run-fast: build
	@echo "Running JStream Benchmark (10000 iterations)..."
	dotnet run --configuration Release --no-build -- 10000

# Run benchmark with custom iterations
# Usage: make run-custom ITERATIONS=50000
run-custom: build
	@echo "Running JStream Benchmark ($(ITERATIONS) iterations)..."
	dotnet run --configuration Release --no-build -- $(ITERATIONS)

# Clean build artifacts
clean:
	@echo "Cleaning JStream Benchmark..."
	dotnet clean
	rm -rf bin/ obj/

# Show help
help:
	@echo "Available targets:"
	@echo "  build      - Build the benchmark project"
	@echo "  run        - Run benchmark with 100000 iterations"
	@echo "  run-fast   - Run benchmark with 10000 iterations"
	@echo "  run-custom - Run benchmark with custom iterations (set ITERATIONS=N)"
	@echo "  clean      - Clean build artifacts"
	@echo "  help       - Show this help message"
