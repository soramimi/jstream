
#include "jstream.h"
#include "test.h"

struct GoogleAccessToken {
	std::string access_token;
	std::string token_type;
	std::string expires_in;
	std::string refresh_token;
	std::string id_token;
};

void main1()
{
	std::string input = R"---(
{
"access_token": "qwerty123",
"expires_in": 3599,
"scope": "https://www.googleapis.com/auth/userinfo.profile",
"token_type": "Bearer",
"id_token": "abcdefg"
}
)---";

	GoogleAccessToken out;
	jstream::Parser json(input.c_str(), input.size());
	while (json.next()) {
		if (json.match("{access_token")) {
			out.access_token = json.string();
		} else if (json.match("{expires_in")) {
			out.expires_in = json.string();
		} else if (json.match("{token_type")) {
			out.token_type = json.string();
		} else if (json.match("{id_token")) {
			out.id_token = json.string();
		}
	}
}

void main2()
{
	static char const input[] = R"---(
{
  "access_token": "qwerty123",
  "expires_in": 3599,
  "scope": "https://www.googleapis.com/auth/userinfo.profile",
  "token_type": "Bearer",
  "id_token": "abcdefg"
}
)---";

	jstream::Parser json(input);
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
}

int main()
{
//	main1();
	test();

	return 0;
}
