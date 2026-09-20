# JStream C# Port

A lightweight, flexible JSON parser and generator library for C#. This is a port of the jstream C++ library.

## Overview

The JStream library provides an event-based JSON parser and a simple interface for generating JSON data. It aims to be a flexible and easy-to-use library for working with JSON in C# applications.

### Key Features

- **Event-based parsing**: Parse JSON data as a stream of events
- **Path-based access**: Access JSON elements using path expressions
- **Flexible configuration**: Supports comments, unquoted keys, trailing commas, hexadecimal numbers, and special constants
- **Culture-independent**: Number parsing is not affected by culture settings
- **Unicode support**: Handles Unicode characters and surrogate pairs in strings
- **Variant-based data model**: Type-safe JSON representation
- **Synchronous with C++ version**: Follows the C++ API and behavior where idiomatically appropriate

## Installation

Add the JStream package to your project:

```bash
dotnet add package JStream
```

Or include the source files directly in your project.

## Usage

### Parsing JSON

```csharp
using JStream;

const string json = """
{
    "name": "John Doe",
    "age": 30,
    "cities": ["New York", "London", "Tokyo"]
}
""";

var reader = new Reader(json);

// Enable optional features if needed
reader.AllowComment = true;
reader.AllowUnquotedKey = true;

// Iterate through JSON events
while (reader.Next())
{
    if (reader.Match("{name") && reader.IsString)
    {
        Console.WriteLine($"Name: {reader.StringValue}");
    }
    else if (reader.Match("{age") && reader.IsNumber)
    {
        Console.WriteLine($"Age: {reader.Number}");
    }
    else if (reader.Match("{cities[*") && reader.IsString)
    {
        Console.WriteLine($"City: {reader.StringValue}");
    }
}
```

### Generating JSON

```csharp
using JStream;

var output = new List<string>();
var writer = new Writer(s => output.Add(s));

// Configure formatting if needed
writer.EnableIndent = true;
writer.EnableNewline = true;

// Generate JSON
writer.Object("", () =>
{
    writer.String("name", "John Doe");
    writer.Number("age", 30);
    writer.Array("cities", () =>
    {
        writer.String("New York");
        writer.String("London");
        writer.String("Tokyo");
    });
    writer.Boolean("active", true);
    writer.Null("optionalField");
});

string json = string.Join("", output);
Console.WriteLine(json);
```

You can also collect output directly in a string:

```csharp
var writer = new Writer();
writer.String("hello", "world");
string json = writer.ToString();
```

Or insert raw JSON text:

```csharp
writer.Raw("metadata", "{\"source\":\"api\"}");
```

### Using the Variant-based API

```csharp
using JStream;

// Create a JSON object
var root = new Variant();
var obj = root.AsObject();

// Add properties
obj["name"] = new Variant("John Doe");
obj["age"] = new Variant(30.0);

// Add an array
var cities = obj["cities"].AsArray();
cities.Add(new Variant("New York"));
cities.Add(new Variant("London"));
cities.Add(new Variant("Tokyo"));

// Access values
if (obj.ContainsKey("name"))
{
    Console.WriteLine($"Name: {obj.Get<string>("name")}");
}
```

### Error Handling

The parser records detailed error information including the message, offset, line, and column:

```csharp
var reader = new Reader(json);
while (reader.Next()) { }

if (reader.HasError)
{
    foreach (var error in reader.Errors)
    {
        Console.WriteLine($"{error.Message} at line {error.Line}, column {error.Column}");
    }
}
```

### Streaming Input

`Reader` can parse JSON that arrives incrementally. Use the constructor that takes an input callback, call `Input(string)` to append data, and check `IsNotEnoughInput` when parsing pauses mid-token.

```csharp
using JStream;

var chunks = new[] {
    "{ \"name\": \"John\", \"a",
    "ge\": 30 }"
};
int index = 0;
Reader? reader = null;
reader = new Reader(() => {
    if (index < chunks.Length) {
        reader!.Input(chunks[index]);
    }
    index++;
});

while (reader.Next()) {
    if (reader.Match("{name") && reader.IsString) {
        Console.WriteLine($"Name: {reader.StringValue}");
    } else if (reader.Match("{age") && reader.IsNumber) {
        Console.WriteLine($"Age: {reader.Number}");
    }
}
```

When a stream contains multiple top-level JSON documents, parse the first document, call `NextDocument()` to clear the `EndDocument` state, and continue parsing.

## Configuration Options

The `Reader` class supports the following configuration options:

- `AllowComment`: Allow C and C++ style comments in JSON
- `AllowAmbiguousComma`: Allow trailing commas in arrays and objects
- `AllowUnquotedKey`: Allow unquoted object keys
- `AllowHexadecimal`: Allow hexadecimal number format (`0xNNN` and `-0xNNN`)
- `AllowSpecialConstant`: Allow special constants like `Infinity` and `NaN`
- `AllowKeyInArray`: Allow `"key":"value"` syntax inside arrays

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

```csharp
reader.Match("{user{name");              // user.name
reader.Match("{items[*{price");          // price inside any object in items
reader.MatchStartObject("{items[*{");    // any object inside items array
reader.Match("{config{**");              // any nested path under config

reader.Nest(() => {                       // enter nested scope
    if (reader.Match("@{value")) {        // match "value" relative to nest baseline
        // ...
    }
});
```

## State Inspection

Useful predicates and accessors on `Reader`:

- `State` - Current state (`StateType`)
- `IsConstant`, `IsStructure`, `IsValue` - Classify the current state (`IsValue = IsConstant || IsStructure`)
- `IsStartObject`, `IsEndObject`, `IsStartArray`, `IsEndArray`, `IsEndDocument`
- `IsNull`, `IsBoolean`, `IsFalse`, `IsTrue`, `IsNumber`, `IsString`
- `Key`, `StringValue`, `Number`, `BooleanValue`
- `Path`, `Depth`, `Tell`
- `Extract()` - Raw text of the last parsed element
- `IsNotEnoughInput` - True when parsing paused because the buffer ended mid-token (streaming mode)

## Building and Testing

A top-level `Makefile` is provided for convenience:

```bash
cd jstream-cs

make              # Build the solution
make test         # Run unit tests
make run          # Run the example application
make benchmark    # Run the benchmark
make clean        # Clean build artifacts
```

Equivalent `dotnet` commands:

```bash
# Build the solution
dotnet build JStreamCSharp.sln

# Run tests
dotnet test JStreamCSharp.sln

# Run examples
dotnet run --project JStream.Examples

# Run benchmark
dotnet run --project JStream.Benchmark
```

## Benchmark

The JStream library includes a comprehensive benchmark that measures parsing performance using complex JSON data. The benchmark parses the same JSON structure as the C++ version and performs the same validation, allowing for direct performance comparison.

```bash
# Run benchmark with default iterations (100,000)
cd jstream-cs/JStream.Benchmark
dotnet run --configuration Release

# Run benchmark with custom iterations
dotnet run --configuration Release -- 50000

# Use Makefile for convenience
cd jstream-cs/JStream.Benchmark
make run          # 100,000 iterations
make run-fast     # 10,000 iterations
make run-custom ITERATIONS=25000
```

### Performance Results

Typical performance on modern hardware:
- **10,000 iterations**: ~1 second
- **100,000 iterations**: ~9-10 seconds

The C# version is typically slower than the C++ version, which is expected for managed vs. native code, while providing additional safety and ease of use.

## Project Structure

- `JStream/` - Main library
  - `Reader.cs` - JSON parser
  - `Writer.cs` - JSON generator
  - `Variant.cs` - Type-safe JSON data model
  - `StateType.cs` - Parser state definitions
  - `JsonHelper.cs` - Utility functions
- `JStream.Tests/` - Unit tests
- `JStream.Examples/` - Usage examples
- `JStream.Benchmark/` - Performance benchmarks
- `Makefile` - Top-level build/test convenience targets

## Ports

- **C++**: the original implementation in `include/jstream.h`. See `README.md`.
- **Go**: `jstream-go/` directory. See `README_Go.md` (also `jstream-go/README.md`).

## Differences from C++ Version

- Uses C# naming conventions (PascalCase for public members)
- Uses `List<T>` and `Dictionary<string, T>` instead of `std::vector` and a linear key-value list
- Error information is exposed through the `Error` class with offset/line/column
- Memory management is automatic (garbage collected)
- String handling uses C#'s built-in UTF-16 Unicode support
- `Writer` accepts `Action<string>` instead of a raw byte callback

## License

This software is distributed under the MIT license, same as the original C++ version.
