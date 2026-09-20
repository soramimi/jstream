[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/soramimi/jstream)

# Jstream

A lightweight, flexible, header-only JSON parser and generator library for C++17.

## Overview

The jstream library provides an event-based streaming JSON parser and a simple interface for generating JSON data. It aims to be a flexible and easy-to-use library for working with JSON in C++ applications.

### Key Features

- **Event-based parsing**: Parse JSON data as a stream of events
- **Path-based access**: Access JSON elements using path expressions
- **Flexible configuration**: Supports comments, unquoted keys, trailing commas, hexadecimal numbers, and special constants
- **Locale-independent**: Number parsing is not affected by locale settings
- **Unicode support**: Handles Unicode characters and surrogate pairs in strings
- **Variant-based data model**: Uses `std::variant` for type-safe JSON representation
- **Header-only**: Just include `include/jstream.h`

## Installation

Just include the header file in your project:

```cpp
#include "jstream.h"
```

## Usage

### Parsing JSON

```cpp
#include "jstream.h"
#include <iostream>

int main() {
    const char* json = R"({
        "name": "John Doe",
        "age": 30,
        "cities": ["New York", "London", "Tokyo"]
    })";

    // Parse JSON
    jstream::Reader reader(json);

    // Enable optional features if needed
    reader.allow_comment(true);
    reader.allow_unquoted_key(true);

    // Iterate through JSON events
    while (reader.next()) {
        if (reader.match("{name") && reader.isstring()) {
            std::cout << "Name: " << reader.string() << std::endl;
        } else if (reader.match("{age") && reader.isnumber()) {
            std::cout << "Age: " << reader.number() << std::endl;
        } else if (reader.match("{cities[*") && reader.isstring()) {
            std::cout << "City: " << reader.string() << std::endl;
        }
    }

    return 0;
}
```

### Generating JSON

```cpp
#include "jstream.h"
#include <iostream>

int main() {
    // Create a JSON writer that outputs to stdout
    jstream::Writer writer([](const char* p, int n) {
        std::cout.write(p, n);
    });

    // Configure formatting if needed
    writer.enable_indent(true);
    writer.enable_newline(true);

    // Generate JSON
    writer.object({}, [&]() {
        writer.string("name", "John Doe");
        writer.number("age", 30);
        writer.array("cities", [&]() {
            writer.string("New York");
            writer.string("London");
            writer.string("Tokyo");
        });
        writer.boolean("active", true);
        writer.null("optionalField");
    });

    return 0;
}
```

You can also collect the output in a `std::string`:

```cpp
jstream::Writer writer;           // no callback
writer.string("hello", "world");
std::string json = writer;        // implicit std::string conversion
```

Or insert raw JSON text:

```cpp
writer.raw("metadata", "{\"source\":\"api\"}");
```

### Using the Variant-based API

```cpp
#include "jstream.h"
#include <iostream>

int main() {
    // Create a JSON object
    jstream::Variant root;
    auto obj = jstream::obj(root);

    // Add properties
    obj["name"] = "John Doe";
    obj["age"] = 30.0;

    // Add an array
    auto& cities = jstream::arr(obj["cities"]);
    cities.push_back("New York");
    cities.push_back("London");
    cities.push_back("Tokyo");

    // Access values
    if (jstream::is_string(obj.value("name"))) {
        std::cout << "Name: " << jstream::get<std::string>(obj.value("name")) << std::endl;
    }

    return 0;
}
```

### Error Handling

The parser records detailed error information including the message, byte offset, line, and column:

```cpp
jstream::Reader reader(json);
while (reader.next()) { }

if (reader.has_error()) {
    for (auto const& e : reader.errors()) {
        std::cout << e.what() << " at line " << e.line
                  << ", column " << e.column << std::endl;
    }
}
```

## Configuration Options

The `Reader` class supports the following configuration options:

- `allow_comment(bool)`: Allow C and C++ style comments in JSON
- `allow_ambiguous_comma(bool)`: Allow trailing commas in arrays and objects
- `allow_unquoted_key(bool)`: Allow unquoted object keys
- `allow_hexadecimal(bool)`: Allow hexadecimal number format (`0xNNN` and `-0xNNN`)
- `allow_special_constant(bool)`: Allow special constants like `Infinity` and `NaN`
- `allow_key_in_array(bool)`: Allow `"key":"value"` syntax inside arrays

## Path Matching

Path expressions provide a concise way to match JSON locations:

- `{key}` - Match an object key
- `[*]` - Match any array element (constant values)
- `{*}` - Match any object key (constant values)
- `*[` - Match any array start
- `*{` - Match any object start
- `**` - Match any nested path (must be at the end)
- `@...` - Relative path from the current `nest()` baseline

Examples:

```cpp
reader.match("{user{name");           // user.name
reader.match("{items[*{price");       // items[].price
reader.match("{items[*{price");       // price inside any object in items
reader.match_start_object("{items[*{"); // any object inside items array

reader.nest([&](){                    // enter nested scope
    if (reader.match("@{value")) {    // match "value" relative to nest baseline
        // ...
    }
});
```

## State Inspection

Useful predicates and accessors on `Reader`:

- `state()` - Current state (`StateType`)
- `is_constant()`, `is_structure()`, `is_value()` - Classify the current state
- `is_start_object()`, `is_end_object()`, `is_start_array()`, `is_end_array()`
- `isnull()`, `isboolean()`, `isnumber()`, `isstring()`
- `key()`, `string()`, `number()`, `boolean()`
- `path()`, `depth()`, `tell()`
- `extract()` - Raw text of the last parsed element

## Building and Testing

A qmake project file is provided for convenience:

```bash
qmake jstream.pro
make
```

The unit tests use Google Test:

```bash
cd test
make
./myapp
```

## Ports

- **C#**: `jstream-cs/` directory. See `README_CSharp.md`.
- **Go**: `jstream-go/` directory. See `README_Go.md`.
- **日本語**: `README_ja.md`

## License

This software is distributed under the MIT license.
