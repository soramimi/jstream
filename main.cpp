
#include "jstream.h"
#include "test.h"
#include <stdio.h>
#include <string_view>
#include <map>
#include <iostream>

struct GoogleAccessToken {
	std::string access_token;
	std::string token_type;
	std::string expires_in;
	std::string scope;
	std::string refresh_token;
	std::string id_token;

	std::string stringify() const
	{
		std::string ret;

		jstream::Writer w([&](char const *p, int n){
			ret += std::string{p, (size_t)n};
		});

		w.object({}, [&](){
			w.string(access_token, "access_token");
			w.string(expires_in, "expires_in");
			w.string(scope, "scope");
			w.string(token_type, "token_type");
			w.string(id_token, "id_token");
		});

		return ret;
	}
};

std::ostream &operator << (std::ostream &out, GoogleAccessToken const &token)
{
	out << token.stringify();
	return out;
}

GoogleAccessToken parse_google_access_token()
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
	jstream::Reader json(input.c_str(), input.size());
	while (json.next()) {
		if (json.match("{access_token")) {
			out.access_token = json.string();
		} else if (json.match("{expires_in")) {
			out.expires_in = json.string();
		} else if (json.match("{scope")) {
			out.scope = json.string();
		} else if (json.match("{token_type")) {
			out.token_type = json.string();
		} else if (json.match("{id_token")) {
			out.id_token = json.string();
		}
	}
	return out;
}

GoogleAccessToken parse_google_access_token2()
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
	jstream::Reader::rule_for_string_t rule;
	jstream::Reader json(input.c_str(), input.size());
	rule["{access_token"] = &out.access_token;
	rule["{expires_in"] = &out.expires_in;
	rule["{scope"] = &out.scope;
	rule["{token_type"] = &out.token_type;
	rule["{id_token"] = &out.id_token;
	json.parse(rule);

	return out;
}

int main()
{
#if 1
	test_all(true);
#elif 1
	main2();
#else
	auto t = parse_google_access_token();
	std::cout << t;
#endif

	return 0;
}
