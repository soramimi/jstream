# JStream C# Port

A lightweight, flexible JSON parser and generator library for C#. This is a port of the jstream C++ library.

## Overview

The JStream library provides an event-based JSON parser and a simple interface for generating JSON data. It aims to be a flexible and easy-to-use library for working with JSON in C# applications.

### Key Features

- **Event-based parsing**: Parse JSON data as a stream of events
- **Path-based access**: Access JSON elements using path expressions
- **Flexible configuration**: Supports comments, unquoted keys, and more
- **Culture-independent**: Number parsing is not affected by culture settings
- **Unicode support**: Handles Unicode characters in strings
- **Variant-based data model**: Uses type-safe JSON representation

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

## Configuration Options

The `Reader` class supports the following configuration options:

- `AllowComment`: Allow C and C++ style comments in JSON
- `AllowAmbiguousComma`: Allow trailing commas in arrays and objects
- `AllowUnquotedKey`: Allow unquoted object keys
- `AllowHexadecimal`: Allow hexadecimal number format (0xNNN)
- `AllowSpecialConstant`: Allow special constants like Infinity and NaN

## Path Matching

JStream supports powerful path-based matching for accessing nested JSON data:

- `{key}` - Match object key
- `[*]` - Match any array element
- `{*}` - Match any object key
- `**` - Match any nested structure

Examples:
```csharp
reader.Match("{user{name}")           // user.name
reader.Match("{items[*{price}")       // items[].price
reader.Match("{config{database{*}")   // any key under config.database
```

## Building and Testing

```bash
# Build the solution
dotnet build

# Run tests
dotnet test

# Run examples
dotnet run --project JStream.Examples
```

## Project Structure

- `JStream/` - Main library
  - `Reader.cs` - JSON parser
  - `Writer.cs` - JSON generator
  - `Variant.cs` - Type-safe JSON data model
  - `StateType.cs` - Parser state definitions
  - `JsonHelper.cs` - Utility functions
- `JStream.Tests/` - Unit tests
- `JStream.Examples/` - Usage examples

## Differences from C++ Version

- Uses C# naming conventions (PascalCase for public members)
- Uses `List<T>` and `Dictionary<string, T>` instead of `std::vector` and `std::map`
- Error handling uses exceptions instead of error codes
- Memory management is automatic (garbage collected)
- String handling uses C#'s built-in Unicode support

## License

This software is distributed under the MIT license, same as the original C++ version.