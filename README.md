[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/soramimi/jstream)

# jstream

A lightweight, flexible JSON parser and generator library for C++.

## Overview

The jstream library provides an event-based JSON parser and a simple interface for generating JSON data. It aims to be a flexible and easy-to-use library for working with JSON in C++ applications.

### Key Features

- **Event-based parsing**: Parse JSON data as a stream of events
- **Path-based access**: Access JSON elements using path expressions
- **Flexible configuration**: Supports comments, unquoted keys, and more
- **Locale-independent**: Number parsing is not affected by locale settings
- **Unicode support**: Handles Unicode characters in strings
- **Variant-based data model**: Uses `std::variant` for type-safe JSON representation

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

## Configuration Options

The `Reader` class supports the following configuration options:

- `allow_comment(bool)`: Allow C and C++ style comments in JSON
- `allow_ambiguous_comma(bool)`: Allow trailing commas in arrays and objects
- `allow_unquoted_key(bool)`: Allow unquoted object keys
- `allow_hexadicimal(bool)`: Allow hexadecimal number format (0xNNN)
- `allow_special_constant(bool)`: Allow special constants like Infinity and NaN

