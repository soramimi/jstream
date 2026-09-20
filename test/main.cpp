
#include "helper.h"
#include <gtest/gtest.h>

void debug1()
{
	char const *json = R"---(
{
	"name": "John",
	"age": 30,
	"city": "New York"
}

{
	"name": "Alice"
	"age": 10,
	"city": "Wonderland"
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
		}
	}
}

int main(int argc, char **argv)
{
	if (1) {
		debug1();
	}
	
	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();
	
	return ret;
}
