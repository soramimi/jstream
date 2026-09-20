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
