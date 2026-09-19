# JStream C# Makefile
# Basic build and test targets for the .NET solution.
#
# Usage:
#   make build      - Build the whole solution
#   make test       - Run all unit tests
#   make run        - Run the example application
#   make benchmark  - Run the benchmark application
#   make clean      - Remove build artifacts

CONFIGURATION ?= Release
SOLUTION := JStreamCSharp.sln

.PHONY: all build test run benchmark clean help

all: build

build:
	dotnet build $(SOLUTION) --configuration $(CONFIGURATION)

test:
	dotnet test $(SOLUTION) --configuration $(CONFIGURATION)

run:
	dotnet run --project JStream.Examples/JStream.Examples.csproj --configuration $(CONFIGURATION)

benchmark:
	dotnet run --project JStream.Benchmark/JStream.Benchmark.csproj --configuration $(CONFIGURATION)

clean:
	-dotnet clean $(SOLUTION) --configuration $(CONFIGURATION)
	rm -rf JStream/bin JStream/obj
	rm -rf JStream.Tests/bin JStream.Tests/obj
	rm -rf JStream.Examples/bin JStream.Examples/obj
	rm -rf JStream.Benchmark/bin JStream.Benchmark/obj

help:
	@echo "Available targets:"
	@echo "  build      - Build the solution (default, CONFIGURATION=Release)"
	@echo "  test       - Run unit tests"
	@echo "  run        - Run the example application"
	@echo "  benchmark  - Run the benchmark application"
	@echo "  clean      - Remove build artifacts"
	@echo "  help       - Show this help message"
