# AGENTS.md

This file contains project-specific context for AI coding agents working on jstream.

## Project Overview

jstream is a lightweight, header-only JSON parser/generator library. It has two implementations:

- **C++ version** (`include/jstream.h`): the primary implementation, written in C++17.
- **C# version** (`jstream-cs/`): a port that follows the C++ API and behavior where idiomatically appropriate.
- **Go version** (`jstream-go/`): a port that follows the C++ API and behavior where idiomatically appropriate.

When modifying functionality, the C++ version is the source of truth. The C# and Go ports should be kept synchronized unless the change is C++-specific.

## Directory Structure

```
.
├── include/jstream.h      # C++ header-only library
├── main.cpp               # C++ sample/entry point (uses test/test.cpp)
├── jstream.pro            # qmake project file for C++ sample
├── test/                  # C++ unit tests using Google Test
│   ├── testmain.cpp
│   ├── test.h
│   ├── test.cpp
│   ├── test1.cpp ... test5.cpp
│   └── Makefile
├── benchmark/             # C++ benchmark
├── jstream-cs/            # C# port
│   ├── JStreamCSharp.sln
│   ├── Makefile           # top-level build/test convenience targets
│   ├── JStream/           # core library
│   ├── JStream.Tests/     # xUnit tests
│   ├── JStream.Examples/  # example application
│   └── JStream.Benchmark/ # benchmark application
├── jstream-go/            # Go port
│   ├── go.mod
│   ├── Makefile
│   ├── reader.go
│   ├── writer.go
│   ├── variant.go
│   ├── state.go
│   ├── helper.go
│   ├── reader_test.go
│   ├── writer_test.go
│   ├── example/
│   ├── benchmark/
│   └── README.md
├── README.md              # C++ documentation
├── README_CSharp.md       # C# documentation
├── README_Go.md           # Go documentation
└── AGENTS.md              # this file
```

## Build and Test

### C++ Version

```bash
# Build sample with qmake
qmake jstream.pro
make

# Run C++ unit tests
cd test
make
./myapp
```

### C# Version

Requires .NET SDK (tested with .NET 10).

```bash
cd jstream-cs
make        # build
make test   # run xUnit tests
make run    # run example
make clean  # clean artifacts
```

### Go Version

Requires Go 1.23 or later.

```bash
cd jstream-go
make        # build and test
make test   # run tests
make example
make benchmark
make clean
```

## Coding Conventions

- **C++**: follow the existing style in `include/jstream.h` (tabs for indentation, snake_case public API).
- **C#**: follow the existing style (PascalCase public API, file-scoped namespace `namespace JStream;`).
- Keep changes minimal and focused.
- Prefer editing existing files over creating new ones unless explicitly required.
- Do not commit unless explicitly asked.

## Important Notes

- `include/jstream.h` is header-only. Any change affects every consumer immediately.
- C++ `Reader` takes input as a non-owning pointer/`std::string_view`. Callers must keep the input string alive for the lifetime of the `Reader`.
- C++ `Writer` can output via callback or collect output internally (since the last sync).
- C# `Reader` takes a managed `string`. It does not require lifetime management.
- C# `Variant` is a reference type (`class`), unlike C++ `std::variant`. `AsObject()`/`AsArray()` mutate the `Variant` instance when the current type does not match.
- Go `Variant` is a value type (`struct`) wrapping `any`. Use pointer methods (`*Variant`) such as `AsObject()`/`AsArray()` when mutating the variant.
- `Reader.IsValue` in all languages means `IsConstant || IsStructure`. Use `IsConstant` when only primitive/null values are intended.

## Synchronization Policy

When adding or changing features:

1. Implement the change in the C++ version first (`include/jstream.h`).
2. Update C++ tests in `test/` as needed.
3. Port the change to the C# version (`jstream-cs/JStream/`).
4. Update C# tests and examples to match.
5. Port the change to the Go version (`jstream-go/`).
6. Update Go tests and examples to match.
7. Update `README.md`, `README_CSharp.md`, `README_Go.md`, and `jstream-go/README.md`.

Keep API names aligned by converting C++ snake_case to C# PascalCase (e.g. `allow_comment` → `AllowComment`, `is_value` → `IsValue`) and to Go PascalCase (e.g. `allow_comment` → `AllowComment`, `is_value` → `IsValue`).

## Pending Ports (Planned for C# and Go)

The following features were recently added to the C++ version and are **not yet ported** to C# and Go. They are planned for future synchronization.

### 1. Streaming Input Mode

Commit: `e59bb26`

- `Reader(std::function<void()> callback)` constructor: invokes the callback when more input is needed.
- `input(std::string_view)` method: appends incremental input to the internal buffer.
- `is_not_enough_input()` method: returns `true` when parsing was interrupted because the buffer ended mid-token.

**Porting notes:**
- **C#**: Add a constructor taking `Action`, an `Input(string)` method, and an `IsNotEnoughInput` property.
- **Go**: Add a constructor taking `func()`, an `Input(string)` method, and an `IsNotEnoughInput()` method.

### 2. Comment-aware Whitespace Skipping in Streaming Mode

Commit: `086a40b`

- Replaced `scan_space()` with `skip_space()`, which returns `bool` and correctly skips `//` and `/* */` comments even when the comment spans across buffer boundaries during streaming input.

**Porting notes:**
- **C# / Go**: Implement a `SkipSpace()` (or `skipSpace()`) helper that mirrors the C++ logic, handling comment state across incremental `input()` calls when `AllowComment` is enabled.

### 3. Multi-Document Support

Commit: `e712d6c`

- Added `StateType::EndDocument` state, emitted when a top-level JSON object/array closes and the parser reaches depth 0.
- Added `next_document()` method: clears the `EndDocument` state so the same `Reader` can parse the next JSON document from the same stream.

**Porting notes:**
- **C# / Go**: Add `EndDocument` to `StateType`, add `NextDocument()` to `Reader`, and update `Next()` to return `true` when `EndDocument` is reached in streaming mode (unless input is insufficient).

### 4. Test Coverage

- C++ tests are in `test/test5.cpp`. When porting, add equivalent streaming / multi-document tests to `JStream.Tests/` (C#) and `reader_test.go` (Go).

## Common Pitfalls

- The C++ `match()` function distinguishes `*` (constants), `*{` / `*` (structure starts), and `**` (any nested path). Porting path matching to C# must preserve these semantics.
- C++ `Object::operator[]` inserts a new key if it does not exist. C# `JObject.this[string]` returns `Variant.Null` for missing keys; use the setter to add or overwrite.
- `Writer` raw JSON insertion is the caller's responsibility to ensure valid JSON.
