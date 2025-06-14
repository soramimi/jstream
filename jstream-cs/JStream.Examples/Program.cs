using System;
using System.Collections.Generic;
using JStream;

namespace JStream.Examples;

class Program {
	static void Main(string[] args)
	{
		Console.WriteLine("JStream C# Examples");
		Console.WriteLine("===================");

		// Example 1: Basic parsing
		Console.WriteLine("\n1. Basic JSON Parsing:");
		BasicParsingExample();

		// Example 2: JSON generation
		Console.WriteLine("\n2. JSON Generation:");
		JsonGenerationExample();

		// Example 3: Complex parsing with arrays and nested objects
		Console.WriteLine("\n3. Complex Parsing Example:");
		ComplexParsingExample();

		// Example 4: Using Variant API
		Console.WriteLine("\n4. Variant API Example:");
		VariantApiExample();

		// Example 5: Print events (like the C++ test)
		Console.WriteLine("\n5. Print Events Example:");
		PrintEventsExample();
	}

	static void BasicParsingExample()
	{
		const string json = """
        {
            "name": "John Doe",
            "age": 30,
            "cities": ["New York", "London", "Tokyo"]
        }
        """;

		var reader = new Reader(json);

		while (reader.Next()) {
			if (reader.Match("{name") && reader.IsString) {
				Console.WriteLine($"Name: {reader.StringValue}");
			} else if (reader.Match("{age") && reader.IsNumber) {
				Console.WriteLine($"Age: {reader.Number}");
			} else if (reader.Match("{cities[*") && reader.IsString) {
				Console.WriteLine($"City: {reader.StringValue}");
			}
		}
	}

	static void JsonGenerationExample()
	{
		var output = new List<string>();
		var writer = new Writer(s => output.Add(s));

		writer.EnableIndent = true;
		writer.EnableNewline = true;

		writer.Object("", () => {
			writer.String("name", "John Doe");
			writer.Number("age", 30);
			writer.Array("cities", () => {
				writer.String("New York");
				writer.String("London");
				writer.String("Tokyo");
			});
			writer.Boolean("active", true);
			writer.Null("optionalField");
		});

		writer.Finish();
		string json = string.Join("", output);
		Console.WriteLine(json);
	}

	static void ComplexParsingExample()
	{
		const string json = """
        {
           "book":[
              {
                 "id":"444",
                 "language":"C",
                 "edition":"First",
                 "author":"Dennis Ritchie"
              },
              {
                 "id":"555",
                 "language":"C++",
                 "edition":"Second",
                 "author":"Bjarne Stroustrup"
              }
           ]
        }
        """;

		var books = new List<Book>();
		var reader = new Reader(json);

		while (reader.Next()) {
			if (reader.Match("{book[*")) {
				var book = new Book();
				reader.Nest();
				do {
					if (reader.Match("{book[{id"))
						book.Id = reader.StringValue;
					else if (reader.Match("{book[{language"))
						book.Language = reader.StringValue;
					else if (reader.Match("{book[{edition"))
						book.Edition = reader.StringValue;
					else if (reader.Match("{book[{author"))
						book.Author = reader.StringValue;
				} while (reader.Next());
				books.Add(book);
			}
		}

		Console.WriteLine("Books:");
		foreach (var book in books) {
			Console.WriteLine($"  {book.Language} ({book.Edition}) by {book.Author}");
		}
	}

	static void VariantApiExample()
	{
		// Create a JSON object using Variant API
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
		if (obj.ContainsKey("name")) {
			Console.WriteLine($"Name: {obj.Get<string>("name")}");
		}

		if (obj.ContainsKey("cities")) {
			var citiesArray = obj["cities"].Get<JArray>();
			Console.WriteLine("Cities:");
			for (int i = 0; i < citiesArray.Count; i++) {
				Console.WriteLine($"  - {citiesArray.Get<string>(i)}");
			}
		}
	}

	static void PrintEventsExample()
	{
		const string json = """
        {
           "book":[
              {
                 "id":"444",
                 "language":"C",
                 "edition":"First",
                 "author":"Dennis Ritchie"
              },
              {
                 "id":"555",
                 "language":"C++",
                 "edition":"Second",
                 "author":"Bjarne Stroustrup"
              }
           ]
        }
        """;

		Console.WriteLine("Events:");
		var reader = new Reader(json);
		var events = new List<(StateType state, string key, string value, string path)>();

		while (reader.Next()) {
			var eventItem = (reader.State, reader.Key, reader.IsValue ? reader.StringValue : "", reader.Path);
			events.Add(eventItem);
		}

		foreach (var evt in events) {
			Console.WriteLine($"{{State={evt.state}, Key=\"{evt.key}\", Value=\"{evt.value}\", Path=\"{evt.path}\"}},");
		}
	}

	public class Book {
		public string Id { get; set; } = string.Empty;
		public string Language { get; set; } = string.Empty;
		public string Edition { get; set; } = string.Empty;
		public string Author { get; set; } = string.Empty;
	}
}