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
    json := `{"name": "John Doe", "age": 30, "cities": ["New York", "London", "Tokyo"]}`
    reader := jstream.NewReader(json)

    reader.AllowComment = true
    reader.AllowUnquotedKey = true

    for reader.Next() {
        if reader.Match("{name") && reader.IsString() {
            fmt.Println(reader.StringValue())
        } else if reader.Match("{age") && reader.IsNumber() {
            fmt.Println(reader.Number())
        } else if reader.Match("{cities[*") && reader.IsString() {
            fmt.Println(reader.StringValue())
        }
    }
}
```

### Generating JSON

```go
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
```

Or insert raw JSON text:

```go
writer.WriteRaw("metadata", `{"source":"api"}`)
```

### Using the Variant-based API

```go
root := jstream.NewVariant()
obj := root.AsObject()

obj.Set("name", jstream.NewString("John Doe"))
obj.Set("age", jstream.NewNumber(30))

cities := obj.Get("cities").AsArray()
cities.Append(jstream.NewString("New York"))
cities.Append(jstream.NewString("London"))

if v, ok := obj.Get("name").TryString(); ok {
    fmt.Println(v)
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

- `{key}` - Match an object key
- `[*]` - Match any array element (constant values)
- `{*}` - Match any object key (constant values)
- `*[` - Match any array start
- `*{` - Match any object start
- `**` - Match any nested path (must be at the end)

Examples:

```go
reader.Match("{user{name")              // user.name
reader.Match("{items[*{price")          // price inside any object in items
reader.MatchStartObject("{items[*{")    // any object inside items array
reader.Match("{config{**")              // any nested path under config
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

## Building and Testing

```bash
make        # build and test
make test   # run tests
make example
make benchmark
make clean  # clean artifacts
```

## Project Structure

- `reader.go` - JSON parser (`Reader`, `Error`)
- `writer.go` - JSON generator (`Writer`)
- `variant.go` - Type-safe JSON data model (`Variant`, `JObject`, `JArray`)
- `state.go` - Parser state definitions (`StateType`)
- `helper.go` - Utility functions
- `reader_test.go`, `writer_test.go` - Unit tests
- `example/` - Usage example
- `benchmark/` - Performance benchmark

## Ports

- **C++**: the original implementation in `include/jstream.h`. See `README.md`.
- **C#**: `jstream-cs/` directory. See `README_CSharp.md`.

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
