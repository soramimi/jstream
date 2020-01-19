
#include "jstream.h"

int main()
{
	static char const source[] = R"---(
"fruits": [
	{"name":"apple","price":150},
	{"name":"banana","price":80},
	{"name":"orange","price":52}
]
)---";

	jstream::Parser json(source, source + strlen(source));
	while (json.next()) {
		switch (json.state()) {
		case jstream::StartObject:
			printf("StartObject: \"%s\"\n", json.key().c_str());
			break;
		case jstream::EndObject:
			printf("EndObject: \"%s\"\n", json.key().c_str());
			break;
		case jstream::StartArray:
			printf("StartArray: \"%s\"\n", json.key().c_str());
			break;
		case jstream::EndArray:
			printf("EndArray: \"%s\"\n", json.key().c_str());
			break;
		case jstream::String:
			printf("StringValue: \"%s\" = \"%s\"\n", json.key().c_str(), json.string().c_str());
			break;
		case jstream::Number:
			printf("NumberValue: \"%s\" = %f\n", json.key().c_str(), json.number());
			break;
		case jstream::Null:
		case jstream::False:
		case jstream::True:
			printf("SymbolValue: \"%s\" = %s\n", json.key().c_str(), json.string().c_str());
			break;
		}
	}

	return 0;
}
