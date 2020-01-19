#include "test.h"
#include "jstream.h"

std::string trim(std::string const &s)
{
	size_t i = 0;
	size_t j = s.size();
	while (i < j && isspace((unsigned char)s[i])) i++;
	while (i < j && isspace((unsigned char)s[j - 1])) j--;
	return s.substr(i, j - i);
}

std::string parse(char const *source)
{
	std::string ret;
	jstream::Parser json(source, source + strlen(source));
	while (json.next()) {
		char tmp[1000];
		tmp[0] = 0;
		switch (json.state()) {
		case jstream::StartObject:
			sprintf(tmp, "StartObject: \"%s\"\n", json.key().c_str());
			break;
		case jstream::EndObject:
			sprintf(tmp, "EndObject: \"%s\"\n", json.key().c_str());
			break;
		case jstream::StartArray:
			sprintf(tmp, "StartArray: \"%s\"\n", json.key().c_str());
			break;
		case jstream::EndArray:
			sprintf(tmp, "EndArray: \"%s\"\n", json.key().c_str());
			break;
		case jstream::String:
			sprintf(tmp, "StringValue: \"%s\" = \"%s\"\n", json.key().c_str(), json.string().c_str());
			break;
		case jstream::Number:
			sprintf(tmp, "NumberValue: \"%s\" = %f\n", json.key().c_str(), json.number());
			break;
		case jstream::Null:
		case jstream::False:
		case jstream::True:
			sprintf(tmp, "SymbolValue: \"%s\" = %s\n", json.key().c_str(), json.string().c_str());
			break;
		}
		ret += tmp;
	}
	if (json.state() == jstream::SyntaxError) {
		ret = json.string();
	}
	return ret;
}

void test()
{
	int pass = 0;
	int fail = 0;
	int total = 0;
	auto TEST = [&](std::string const &input, std::string const &expected){
		total++;
		std::string result = parse(input.c_str());
		if (trim(result) == trim(expected)) {
			printf("#%d PASS\n", total);
			pass++;
		} else {
			printf("#%d FAIL\n", total);
			puts("--- INPUT ---------");
			puts(trim(input).c_str());
			puts("--- EXPECTED ----------");
			puts(trim(expected).c_str());
			puts("--- ACTUAL RESULT ----------");
			puts(trim(result).c_str());
			puts("---");
			fail++;
		}
	};


	TEST(
				R"---(
{
	"string": "abc",
	"number": 123.456789,
	"null": null,
	"false": false,
	"true": true
}
				)---",
				R"---(
StartObject: ""
StringValue: "string" = "abc"
NumberValue: "number" = 123.456789
SymbolValue: "null" = null
SymbolValue: "false" = false
SymbolValue: "true" = true
EndObject: ""
				)---");

	TEST(
				R"---(
{
	"a": [ 12, 34, 56, 78 ]
}
				)---",
				R"---(
StartObject: ""
StartArray: "a"
NumberValue: "" = 12.000000
NumberValue: "" = 34.000000
NumberValue: "" = 56.000000
NumberValue: "" = 78.000000
EndArray: "a"
EndObject: ""
				)---");


	TEST(
				R"---(
{
	"a": [[ 12, 34 ],[ 56, 78 ]]
}
				)---",
				R"---(
StartObject: ""
StartArray: "a"
StartArray: ""
NumberValue: "" = 12.000000
NumberValue: "" = 34.000000
EndArray: ""
StartArray: ""
NumberValue: "" = 56.000000
NumberValue: "" = 78.000000
EndArray: ""
EndArray: "a"
EndObject: ""
				)---");

	TEST(
				R"---(
"fruits": [
	{ "name": "apple", "price": 150 },
	{ "name": "banana", "price": 80 },
	{ "name": "orange", "price": 52 }
]
				)---",
				R"---(
StartArray: "fruits"
StartObject: ""
StringValue: "name" = "apple"
NumberValue: "price" = 150.000000
EndObject: ""
StartObject: ""
StringValue: "name" = "banana"
NumberValue: "price" = 80.000000
EndObject: ""
StartObject: ""
StringValue: "name" = "orange"
NumberValue: "price" = 52.000000
EndObject: ""
EndArray: "fruits"
				)---");


	printf("---\n" "TOTAL: %d\n" " PASS: %d\n" " FAIL: %d\n", total, pass, fail);
}


