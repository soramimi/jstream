#include "helper.h"
#include <gtest/gtest.h>

using namespace jstream;

TEST(Json, Streaming0)
{
	char const *json1 = R"---(
{
	"name": "John",
	"age": 30,
	"city": "New Yo)---";
	
	char const *json2 = R"---(rk",
		"address": {
			"street": "123 Main St",
			"zip": "10001"
		}
}
)---";
	struct ParsedData {
		std::string name;
		int age = 0;
		std::string city;
		struct Address {
			std::string street;
			std::string zip;
		} address;
	} parsed;
	
	jstream::Reader reader;
	reader.input(json1);
	reader.input(json2);
	while (reader.next()) {
		if (reader.match("{name")) {
			parsed.name = reader.string();
		} else if (reader.match("{age")) {
			parsed.age = reader.number();
		} else if (reader.match("{city")) {
			parsed.city = reader.string();
		} else if (reader.match("{address{street")) {
			parsed.address.street = reader.string();
		} else if (reader.match("{address{zip")) {
			parsed.address.zip = reader.string();
		}
	}
	EXPECT_EQ(parsed.name, "John");
	EXPECT_EQ(parsed.age, 30);
	EXPECT_EQ(parsed.city, "New York");
	EXPECT_EQ(parsed.address.street, "123 Main St");
	EXPECT_EQ(parsed.address.zip, "10001");
}

TEST(Json, Streaming1)
{
	char const *json = R"---(
{
	"name": "John",
	"age": 3)---"; // break in the middle of a number
	
	struct ParsedData {
		std::string name;
		int age = 0;
		std::string city;
		struct Address {
			std::string street;
			std::string zip;
		} address;
	} parsed;
	
	jstream::Reader reader;
	reader.input(json);
	while (reader.next()) {
		if (reader.match("{name")) {
			parsed.name = reader.string();
		} else if (reader.match("{age")) {
			parsed.age = reader.number();
		} else if (reader.match("{city")) {
			parsed.city = reader.string();
		} else if (reader.match("{address{street")) {
			parsed.address.street = reader.string();
		} else if (reader.match("{address{zip")) {
			parsed.address.zip = reader.string();
		}
	}
	EXPECT_EQ(parsed.name, "John");
	EXPECT_EQ(parsed.age, 0);
	EXPECT_TRUE(reader.is_not_enough_input());
}

TEST(Json, Streaming2)
{
	char const *json = R"---(
{
	"name": "John",
	"age": 30,
	"city": "New Yo)---"; // break in the middle of a string
	
	struct ParsedData {
		std::string name;
		int age = 0;
		std::string city;
		struct Address {
			std::string street;
			std::string zip;
		} address;
	} parsed;
	
	jstream::Reader reader;
	reader.input(json);
	while (reader.next()) {
		if (reader.match("{name")) {
			parsed.name = reader.string();
		} else if (reader.match("{age")) {
			parsed.age = reader.number();
		} else if (reader.match("{city")) {
			parsed.city = reader.string();
		} else if (reader.match("{address{street")) {
			parsed.address.street = reader.string();
		} else if (reader.match("{address{zip")) {
			parsed.address.zip = reader.string();
		}
	}
	EXPECT_EQ(parsed.name, "John");
	EXPECT_EQ(parsed.age, 30);
	EXPECT_EQ(parsed.city, "");
	EXPECT_TRUE(reader.is_not_enough_input());
}

TEST(Json, Streaming3)
{
	char const *json = R"---(
{
	"name": "John",
	"age": 30,
	"city": "New York",
	"address": {
		"street": "123 Main St",
		"zip": "10001"
	}
}
)---";
	
	struct ParsedData {
		std::string name;
		int age = 0;
		std::string city;
		struct Address {
			std::string street;
			std::string zip;
		} address;
	} parsed;
	
	size_t offset = 0;
	
	jstream::Reader reader([&](){
		reader.input(std::string_view(json + offset, 1));
		offset++;
	});
	while (reader.next()) {
		if (reader.match("{name")) {
			parsed.name = reader.string();
		} else if (reader.match("{age")) {
			parsed.age = reader.number();
		} else if (reader.match("{city")) {
			parsed.city = reader.string();
		} else if (reader.match("{address{street")) {
			parsed.address.street = reader.string();
		} else if (reader.match("{address{zip")) {
			parsed.address.zip = reader.string();
		}
	}
	EXPECT_EQ(parsed.name, "John");
	EXPECT_EQ(parsed.age, 30);
	EXPECT_EQ(parsed.city, "New York");
	EXPECT_EQ(parsed.address.street, "123 Main St");
	EXPECT_EQ(parsed.address.zip, "10001");
}

TEST(Json, Streaming4)
{
	char const *json = R"---(
{
	"name": "John",
	"age": 30,
	"city": "New York", // comment1
	"address": {
		"street": "123 Main St", /* comment2 */
		"zip": "10001"
	}
}
)---";
	
	struct ParsedData {
		std::string name;
		int age = 0;
		std::string city;
		struct Address {
			std::string street;
			std::string zip;
		} address;
	} parsed;
	
	size_t offset = 0;
	
	jstream::Reader reader([&](){
		reader.input(std::string_view(json + offset, 1));
		offset++;
	});
	reader.allow_comment(true);
	while (reader.next()) {
		if (reader.match("{name")) {
			parsed.name = reader.string();
		} else if (reader.match("{age")) {
			parsed.age = reader.number();
		} else if (reader.match("{city")) {
			parsed.city = reader.string();
		} else if (reader.match("{address{street")) {
			parsed.address.street = reader.string();
		} else if (reader.match("{address{zip")) {
			parsed.address.zip = reader.string();
		}
	}
	EXPECT_EQ(parsed.name, "John");
	EXPECT_EQ(parsed.age, 30);
	EXPECT_EQ(parsed.city, "New York");
	EXPECT_EQ(parsed.address.street, "123 Main St");
	EXPECT_EQ(parsed.address.zip, "10001");
}

TEST(Json, Streaming5)
{
	char const *json = R"---(
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
)---"; // two JSON objects in a single stream
	
	struct ParsedData {
		std::string name;
		int age = 0;
		std::string city;
		struct Address {
			std::string street;
			std::string zip;
		} address;
	} parsed;
	
	size_t offset = 0;
	
	jstream::Reader reader([&](){
		reader.input(std::string_view(json + offset, 1));
		offset++;
	});
	
	auto Parse = [&](){
		while (reader.next()) {
			if (reader.match("{name")) {
				parsed.name = reader.string();
			} else if (reader.match("{age")) {
				parsed.age = reader.number();
			} else if (reader.match("{city")) {
				parsed.city = reader.string();
			}
		}
	};
	
	Parse();
	EXPECT_EQ(parsed.name, "John");
	EXPECT_EQ(parsed.age, 30);
	EXPECT_EQ(parsed.city, "New York");
	
	reader.next_document();
	
	Parse();
	EXPECT_EQ(parsed.name, "Alice");
	EXPECT_EQ(parsed.age, 10);
	EXPECT_EQ(parsed.city, "Wonderland");
}

TEST(Json, StreamingCommentSpanChunks)
{
	// Block comment split across two input() calls.
	{
		jstream::Reader reader;
		reader.allow_comment(true);
		reader.input(R"({"a": /* comm)");
		reader.input(R"(ent */ 1})");
		int a = 0;
		while (reader.next()) {
			if (reader.match("{a") && reader.isnumber()) {
				a = (int)reader.number();
			}
		}
		EXPECT_EQ(a, 1);
		EXPECT_FALSE(reader.has_error());
	}

	// Line comment split across two input() calls.
	{
		jstream::Reader reader;
		reader.allow_comment(true);
		reader.input("{\"a\": 1 // first lin");
		reader.input("e\n,\"b\":2}\n");
		int a = 0, b = 0;
		while (reader.next()) {
			if (reader.match("{a") && reader.isnumber()) {
				a = (int)reader.number();
			} else if (reader.match("{b") && reader.isnumber()) {
				b = (int)reader.number();
			}
		}
		EXPECT_EQ(a, 1);
		EXPECT_EQ(b, 2);
		EXPECT_FALSE(reader.has_error());
	}
}

TEST(Json, MalformedNumbers)
{
	auto parse = [](char const *json) {
		jstream::Reader reader(json);
		while (reader.next()) { }
		return reader.has_error();
	};

	EXPECT_TRUE(parse(R"({"a": 1.2.3})"));
	EXPECT_TRUE(parse(R"({"a": 1e})"));
	EXPECT_TRUE(parse(R"({"a": ++1})"));
	EXPECT_TRUE(parse(R"({"a": 1e10e20})"));
	EXPECT_TRUE(parse(R"({"a": -})"));
	EXPECT_TRUE(parse(R"({"a": +1})"));

	// Valid numbers must still parse.
	auto parse_value = [](char const *json) {
		jstream::Reader reader(json);
		double value = 0;
		while (reader.next()) {
			if (reader.match("{a") && reader.isnumber()) {
				value = reader.number();
			}
		}
		return value;
	};

	EXPECT_DOUBLE_EQ(parse_value(R"({"a": 0})"), 0);
	EXPECT_DOUBLE_EQ(parse_value(R"({"a": -0})"), 0);
	EXPECT_DOUBLE_EQ(parse_value(R"({"a": 123})"), 123);
	EXPECT_DOUBLE_EQ(parse_value(R"({"a": -123})"), -123);
	EXPECT_DOUBLE_EQ(parse_value(R"({"a": 1.5})"), 1.5);
	EXPECT_DOUBLE_EQ(parse_value(R"({"a": -1.5})"), -1.5);
	EXPECT_DOUBLE_EQ(parse_value(R"({"a": 1e10})"), 1e10);
	EXPECT_DOUBLE_EQ(parse_value(R"({"a": 1E10})"), 1E10);
	EXPECT_DOUBLE_EQ(parse_value(R"({"a": 1e+10})"), 1e+10);
	EXPECT_DOUBLE_EQ(parse_value(R"({"a": 1e-10})"), 1e-10);
	EXPECT_DOUBLE_EQ(parse_value(R"({"a": 1.5e3})"), 1.5e3);
}

TEST(Json, UnquotedKeyWithWhitespace)
{
	jstream::Reader reader("{key   : 1, another\t\t:\t\"two\"}");
	reader.allow_unquoted_key(true);
	int one = 0;
	std::string two;
	while (reader.next()) {
		if (reader.match("{key") && reader.isnumber()) {
			one = (int)reader.number();
		} else if (reader.match("{another") && reader.isstring()) {
			two = reader.string();
		}
	}
	EXPECT_EQ(one, 1);
	EXPECT_EQ(two, "two");
	EXPECT_FALSE(reader.has_error());
}

TEST(Json, HexadecimalOverflow)
{
	{
		jstream::Reader reader(R"({"a": 0xFFFFFFFFFFFFFFFFFFFFFF})");
		reader.allow_hexadecimal(true);
		while (reader.next()) { }
		EXPECT_TRUE(reader.has_error());
	}
	{
		jstream::Reader reader(R"({"a": 0x7FFFFFFFFFFFFFFF})");
		reader.allow_hexadecimal(true);
		double value = 0;
		while (reader.next()) {
			if (reader.match("{a") && reader.isnumber()) {
				value = reader.number();
			}
		}
		EXPECT_FALSE(reader.has_error());
		EXPECT_DOUBLE_EQ(value, static_cast<double>(LLONG_MAX));
	}
}

TEST(Json, ObjectDuplicateKey)
{
	jstream::Variant root;
	auto obj = jstream::obj(root);
	obj["a"] = 1.0;
	obj["a"] = 2.0;
	EXPECT_EQ(obj.size(), 1u);
	EXPECT_DOUBLE_EQ(jstream::get<double>(obj.value("a")), 2.0);
}
