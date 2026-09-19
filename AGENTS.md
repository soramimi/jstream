# AGENTS.md

This file contains project-specific context for AI coding agents working on jstream.

## Project Overview

jstream is a lightweight, header-only JSON parser/generator library. It has two implementations:

- **C++ version** (`include/jstream.h`): the primary implementation, written in C++17.
- **C# version** (`jstream-cs/`): a port that follows the C++ API and behavior where idiomatically appropriate.

When modifying functionality, the C++ version is the source of truth. The C# port should be kept synchronized unless the change is C++-specific.

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
│   ├── test1.cpp ... test4.cpp
│   └── Makefile
├── benchmark/             # C++ benchmark
├── jstream-cs/            # C# port
│   ├── JStreamCSharp.sln
│   ├── Makefile           # top-level build/test convenience targets
│   ├── JStream/           # core library
│   ├── JStream.Tests/     # xUnit tests
│   ├── JStream.Examples/  # example application
│   └── JStream.Benchmark/ # benchmark application
├── README.md              # C++ documentation
├── README_CSharp.md       # C# documentation
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
- `Reader.IsValue` in both languages means `IsConstant || IsStructure`. Use `IsConstant` when only primitive/null values are intended.

## Synchronization Policy

When adding or changing features:

1. Implement the change in the C++ version first (`include/jstream.h`).
2. Update C++ tests in `test/` as needed.
3. Port the change to the C# version (`jstream-cs/JStream/`).
4. Update C# tests and examples to match.
5. Update `README.md` and `README_CSharp.md`.

Keep API names aligned by converting C++ snake_case to C# PascalCase (e.g. `allow_comment` → `AllowComment`, `is_value` → `IsValue`).

## Common Pitfalls

- The C++ `match()` function distinguishes `*` (constants), `*{` / `*` (structure starts), and `**` (any nested path). Porting path matching to C# must preserve these semantics.
- C++ `Object::operator[]` inserts a new key if it does not exist. C# `JObject.this[string]` returns `Variant.Null` for missing keys; use the setter to add or overwrite.
- `Writer` raw JSON insertion is the caller's responsibility to ensure valid JSON.
