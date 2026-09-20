# JStream Go Port

A lightweight, flexible JSON parser and generator library for Go. This is a port of the jstream C++ library.

## Overview

The JStream library provides an event-based JSON parser and a simple interface for generating JSON data. It aims to be a flexible and easy-to-use library for working with JSON in Go applications.

### Key Features

- **Event-based parsing**: Parse JSON data as a stream of events
- **Path-based access**: Access JSON elements using path expressions
- **Flexible configuration**: Supports comments, unquoted keys, trailing commas, hexadecimal numbers, and special constants
- **Locale-independent**: Number parsing uses `strconv.ParseFloat`
- **Unicode support**: Handles Unicode characters and surrogate pairs in strings
- **Variant-based data model**: Type-safe JSON representation using `Variant`
- **Synchronous with C++ version**: Follows the C++ API and behavior where idiomatically appropriate

## Installation

```bash
go get github.com/soramimi/jstream/jstream-go
```

## Usage

### Parsing JSON

```go
package main

import (
    "fmt"
    "github.com/soramimi/jstream/jstream-go"
)

func main() {
    json := `{
        "name": "John Doe",
        "age": 30,
        "cities": ["New York", "London", "Tokyo"]
    }`

    reader := jstream.NewReader(json)

    // Enable optional features if needed
    reader.AllowComment = true
    reader.AllowUnquotedKey = true

    // Iterate through JSON events
    for reader.Next() {
        if reader.Match("{name") && reader.IsString() {
            fmt.Println("Name:", reader.StringValue())
        } else if reader.Match("{age") && reader.IsNumber() {
            fmt.Println("Age:", reader.Number())
        } else if reader.Match("{cities[*") && reader.IsString() {
            fmt.Println("City:", reader.StringValue())
        }
    }
}
```

### Streaming Input

For incremental or callback-driven parsing, create a reader with an input callback or feed chunks via `Input()`:

```go
// Callback-driven streaming
offset := 0
reader := jstream.NewReaderWithCallback(func() {
    if offset < len(json) {
        reader.Input(json[offset : offset+1])
        offset++
    }
})

// Or push chunks manually
reader := jstream.NewReader("")
reader.Input(`{"name": "Jo`)
reader.Input(`hn"}`)
```

In streaming mode:

- `IsNotEnoughInput()` reports when parsing paused because a token was split across chunks.
- `Input()` resets the not-enough-input flag and appends the new chunk to the remaining buffer.
- `Extract()` and `ExtractRange()` are disabled in streaming mode and will record an error.
- When a top-level object or array completes, the reader enters `StateEndDocument`. Call `NextDocument()` to continue parsing the next JSON document from the same stream.

### Generating JSON

```go
package main

import (
    "fmt"
    "github.com/soramimi/jstream/jstream-go"
)

func main() {
    writer := jstream.NewWriter()
    writer.EnableIndent(true)
    writer.EnableNewline(true)

    writer.Object("", func() {
        writer.WriteString("name", "John Doe")
        writer.WriteNumber("age", 30)
        writer.Array("cities", func() {
            writer.WriteStringValue("New York")
            writer.WriteStringValue("London")
            writer.WriteStringValue("Tokyo")
        })
        writer.WriteBoolean("active", true)
        writer.WriteNull("optionalField")
    })

    fmt.Println(writer.String())
}
```

You can also write output directly via a callback:

```go
writer := jstream.NewWriterWithOutput(func(chunk string) {
    fmt.Print(chunk)
})
```

Or insert raw JSON text:

```go
writer.WriteRaw("metadata", `{"source":"api"}`)
```

### Using the Variant-based API

```go
package main

import (
    "fmt"
    "github.com/soramimi/jstream/jstream-go"
)

func main() {
    root := jstream.NewVariant()
    obj := root.AsObject()

    obj.Set("name", jstream.NewString("John Doe"))
    obj.Set("age", jstream.NewNumber(30))

    cities := obj.Get("cities").AsArray()
    cities.Append(jstream.NewString("New York"))
    cities.Append(jstream.NewString("London"))
    cities.Append(jstream.NewString("Tokyo"))

    if v, ok := obj.Get("name").TryString(); ok {
        fmt.Println("Name:", v)
    }
}
```

### Error Handling

The parser records detailed error information including the message, byte offset, line, and column:

```go
reader := jstream.NewReader(json)
for reader.Next() { }

if reader.HasError() {
    for _, e := range reader.Errors() {
        fmt.Printf("%s at line %d, column %d\n", e.Message, e.Line, e.Column)
    }
}
```

## Configuration Options

The `Reader` struct exposes the following configuration fields:

- `AllowComment`: Allow C and C++ style comments in JSON
- `AllowAmbiguousComma`: Allow trailing commas in arrays and objects
- `AllowUnquotedKey`: Allow unquoted object keys
- `AllowHexadecimal`: Allow hexadecimal number format (`0xNNN` and `-0xNNN`)
- `AllowSpecialConstant`: Allow special constants like `Infinity` and `NaN`
- `AllowKeyInArray`: Allow `"key":"value"` syntax inside arrays

The `Writer` supports:

- `EnableIndent(bool)`: Pretty-print with two-space indentation
- `EnableNewline(bool)`: Insert newlines between elements
- `AllowNaN(bool)`: Allow `NaN` and `Infinity` as number output

## Path Matching

Path expressions provide a concise way to match JSON locations:

- `{key}` - Match an object key
- `[*]` - Match any array element (constant values)
- `{*}` - Match any object key (constant values)
- `*[` - Match any array start
- `*{` - Match any object start
- `**` - Match any nested path (must be at the end)
- `@...` - Relative path from the current `Nest()` baseline

Examples:

```go
reader.Match("{user{name")              // user.name
reader.Match("{items[*{price")          // price inside any object in items
reader.MatchStartObject("{items[*{")    // any object inside items array
reader.Match("{config{**")              // any nested path under config

reader.Nest(func() {                    // enter nested scope
    if reader.Match("@{value") {        // match "value" relative to nest baseline
        // ...
    }
})
```

## State Inspection

Useful predicates and accessors on `Reader`:

- `State()` - Current state (`StateType`)
- `IsConstant()`, `IsStructure()`, `IsValue()` - Classify the current state (`IsValue = IsConstant || IsStructure`)
- `IsStartObject()`, `IsEndObject()`, `IsStartArray()`, `IsEndArray()`
- `IsNull()`, `IsFalse()`, `IsTrue()`, `IsBoolean()`, `IsNumber()`, `IsString()`
- `Key()`, `StringValue()`, `Number()`, `BooleanValue()`
- `Path()`, `Depth()`, `Tell()`
- `Extract()` - Raw text of the last parsed element
- `StateEndDocument` - Emitted when a top-level object/array closes in streaming mode
- `IsNotEnoughInput()` - True when parsing paused for more input in streaming mode
- `NextDocument()` - Clear `StateEndDocument` to parse the next document from the stream

## Building and Testing

A top-level `Makefile` is provided for convenience:

```bash
cd jstream-go

make        # build and test
make test   # run tests
make example
make benchmark
make clean  # clean artifacts
```

Equivalent `go` commands:

```bash
# Run tests
go test ./...

# Run example
cd example
go run .

# Run benchmark
cd benchmark
go run .
```

## Benchmark

The Go port includes a benchmark that parses the same complex JSON as the C++ benchmark and performs the same validation, allowing for direct performance comparison.

```bash
cd jstream-go/benchmark
go run .
```

Or use the Makefile:

```bash
cd jstream-go
make benchmark
```

The default run parses the JSON 10,000 times.

## Project Structure

- `reader.go` - JSON parser (`Reader`, `Error`)
- `writer.go` - JSON generator (`Writer`)
- `variant.go` - Type-safe JSON data model (`Variant`, `JObject`, `JArray`)
- `state.go` - Parser state definitions (`StateType`)
- `helper.go` - Utility functions
- `reader_test.go`, `writer_test.go` - Unit tests
- `example/` - Usage example
- `benchmark/` - Performance benchmark
- `Makefile` - Top-level build/test convenience targets
- `README_Go.md` - This document

## Ports

- **C++**: the original implementation in `include/jstream.h`. See `README.md`.
- **C#**: `jstream-cs/` directory. See `README_CSharp.md`.
- **日本語**: See `README_ja.md`.

## Differences from C++ Version

- Uses Go naming conventions (PascalCase for exported members)
- Uses `map[string]Variant` for objects and slices for arrays
- Error information is exposed through the `Error` struct with `Offset`/`Line`/`Column`
- Memory management is automatic (garbage collected)
- String handling uses Go's built-in UTF-8 support
- `Writer` accepts `func(string)` instead of a raw byte callback
- `Variant.AsObject()` and `AsArray()` use pointer receivers to mutate the value when the current type does not match

## License

This software is distributed under the MIT license, same as the original C++ version.
