using System;
using Xunit;

namespace JStream.Tests;

public class StreamingTests {
	private class ParsedData {
		public string Name { get; set; } = string.Empty;
		public int Age { get; set; }
		public string City { get; set; } = string.Empty;
		public AddressData Address { get; set; } = new();
	}

	private class AddressData {
		public string Street { get; set; } = string.Empty;
		public string Zip { get; set; } = string.Empty;
	}

	[Fact]
	public void Streaming0_Test()
	{
		const string json1 = """
			{
				"name": "John",
				"age": 30,
				"city": "New Yo
			""";
		const string json2 = """
			rk",
				"address": {
					"street": "123 Main St",
					"zip": "10001"
				}
			}
			""";

		var parsed = new ParsedData();
		var reader = new Reader();
		reader.Input(json1);
		reader.Input(json2);

		while (reader.Next()) {
			if (reader.Match("{name")) {
				parsed.Name = reader.StringValue;
			} else if (reader.Match("{age")) {
				parsed.Age = (int)reader.Number;
			} else if (reader.Match("{city")) {
				parsed.City = reader.StringValue;
			} else if (reader.Match("{address{street")) {
				parsed.Address.Street = reader.StringValue;
			} else if (reader.Match("{address{zip")) {
				parsed.Address.Zip = reader.StringValue;
			}
		}

		Assert.Equal("John", parsed.Name);
		Assert.Equal(30, parsed.Age);
		Assert.Equal("New York", parsed.City);
		Assert.Equal("123 Main St", parsed.Address.Street);
		Assert.Equal("10001", parsed.Address.Zip);
	}

	[Fact]
	public void Streaming1_Test()
	{
		const string json = """
			{
				"name": "John",
				"age": 3
			""";

		var parsed = new ParsedData();
		var reader = new Reader();
		reader.Input(json);

		while (reader.Next()) {
			if (reader.Match("{name")) {
				parsed.Name = reader.StringValue;
			} else if (reader.Match("{age")) {
				parsed.Age = (int)reader.Number;
			} else if (reader.Match("{city")) {
				parsed.City = reader.StringValue;
			}
		}

		Assert.Equal("John", parsed.Name);
		Assert.Equal(0, parsed.Age);
		Assert.True(reader.IsNotEnoughInput);
	}

	[Fact]
	public void Streaming2_Test()
	{
		const string json = """
			{
				"name": "John",
				"age": 30,
				"city": "New Yo
			""";

		var parsed = new ParsedData();
		var reader = new Reader();
		reader.Input(json);

		while (reader.Next()) {
			if (reader.Match("{name")) {
				parsed.Name = reader.StringValue;
			} else if (reader.Match("{age")) {
				parsed.Age = (int)reader.Number;
			} else if (reader.Match("{city")) {
				parsed.City = reader.StringValue;
			}
		}

		Assert.Equal("John", parsed.Name);
		Assert.Equal(30, parsed.Age);
		Assert.Equal(string.Empty, parsed.City);
		Assert.True(reader.IsNotEnoughInput);
	}

	[Fact]
	public void Streaming3_Test()
	{
		const string json = """
			{
				"name": "John",
				"age": 30,
				"city": "New York",
				"address": {
					"street": "123 Main St",
					"zip": "10001"
				}
			}
			""";

		var parsed = new ParsedData();
		Reader? reader = null;
		int offset = 0;
		reader = new Reader(() => {
			if (offset < json.Length) {
				reader!.Input(json.Substring(offset, 1));
			}
			offset++;
		});

		while (reader.Next()) {
			if (reader.Match("{name")) {
				parsed.Name = reader.StringValue;
			} else if (reader.Match("{age")) {
				parsed.Age = (int)reader.Number;
			} else if (reader.Match("{city")) {
				parsed.City = reader.StringValue;
			} else if (reader.Match("{address{street")) {
				parsed.Address.Street = reader.StringValue;
			} else if (reader.Match("{address{zip")) {
				parsed.Address.Zip = reader.StringValue;
			}
		}

		Assert.Equal("John", parsed.Name);
		Assert.Equal(30, parsed.Age);
		Assert.Equal("New York", parsed.City);
		Assert.Equal("123 Main St", parsed.Address.Street);
		Assert.Equal("10001", parsed.Address.Zip);
	}

	[Fact]
	public void Streaming4_Test()
	{
		const string json = """
			{
				"name": "John",
				"age": 30,
				"city": "New York", // comment1
				"address": {
					"street": "123 Main St", /* comment2 */
					"zip": "10001"
				}
			}
			""";

		var parsed = new ParsedData();
		Reader? reader = null;
		int offset = 0;
		reader = new Reader(() => {
			if (offset < json.Length) {
				reader!.Input(json.Substring(offset, 1));
			}
			offset++;
		});
		reader.AllowComment = true;

		while (reader.Next()) {
			if (reader.Match("{name")) {
				parsed.Name = reader.StringValue;
			} else if (reader.Match("{age")) {
				parsed.Age = (int)reader.Number;
			} else if (reader.Match("{city")) {
				parsed.City = reader.StringValue;
			} else if (reader.Match("{address{street")) {
				parsed.Address.Street = reader.StringValue;
			} else if (reader.Match("{address{zip")) {
				parsed.Address.Zip = reader.StringValue;
			}
		}

		Assert.Equal("John", parsed.Name);
		Assert.Equal(30, parsed.Age);
		Assert.Equal("New York", parsed.City);
		Assert.Equal("123 Main St", parsed.Address.Street);
		Assert.Equal("10001", parsed.Address.Zip);
	}

	[Fact]
	public void Streaming5_Test()
	{
		const string json = """
			{
				"name": "John",
				"age": 30,
				"city": "New York"
			}

			{
				"name": "Alice",
				"age": 10,
				"city": "Wonderland"
			}
			""";

		var parsed = new ParsedData();
		Reader? reader = null;
		int offset = 0;
		reader = new Reader(() => {
			if (offset < json.Length) {
				reader!.Input(json.Substring(offset, 1));
			}
			offset++;
		});

		void Parse()
		{
			while (reader!.Next()) {
				if (reader.Match("{name")) {
					parsed.Name = reader.StringValue;
				} else if (reader.Match("{age")) {
					parsed.Age = (int)reader.Number;
				} else if (reader.Match("{city")) {
					parsed.City = reader.StringValue;
				}
			}
		}

		Parse();
		Assert.Equal("John", parsed.Name);
		Assert.Equal(30, parsed.Age);
		Assert.Equal("New York", parsed.City);

		reader.NextDocument();

		Parse();
		Assert.Equal("Alice", parsed.Name);
		Assert.Equal(10, parsed.Age);
		Assert.Equal("Wonderland", parsed.City);
	}

	[Fact]
	public void StreamingCommentSpanChunks_Test()
	{
		// Block comment split across two Input() calls.
		{
			var reader = new Reader();
			reader.AllowComment = true;
			reader.Input("""{"a": /* comm""");
			reader.Input("""ent */ 1}""");
			int a = 0;
			while (reader.Next()) {
				if (reader.Match("{a") && reader.IsNumber) {
					a = (int)reader.Number;
				}
			}
			Assert.Equal(1, a);
			Assert.False(reader.HasError);
		}

		// Line comment split across two Input() calls.
		{
			var reader = new Reader();
			reader.AllowComment = true;
			reader.Input("{\"a\": 1 // first lin");
			reader.Input("e\n,\"b\":2}\n");
			int a = 0, b = 0;
			while (reader.Next()) {
				if (reader.Match("{a") && reader.IsNumber) {
					a = (int)reader.Number;
				} else if (reader.Match("{b") && reader.IsNumber) {
					b = (int)reader.Number;
				}
			}
			Assert.Equal(1, a);
			Assert.Equal(2, b);
			Assert.False(reader.HasError);
		}
	}
}
