
#include "jstream.h"
#include "test.h"
#include <stdio.h>
#include <map>
#include <iostream>
#include <assert.h>


// testing google access token

std::ostream &operator << (std::ostream &out, GoogleAccessToken const &token)
{
	out << token.stringify();
	return out;
}


GoogleAccessToken test_google_access_token()
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
	jstream::Reader reader(input);
	while (reader.next()) {
		if (reader.match("{access_token")) {
			out.access_token = reader.string();
		} else if (reader.match("{expires_in")) {
			out.expires_in = reader.string();
		} else if (reader.match("{scope")) {
			out.scope = reader.string();
		} else if (reader.match("{token_type")) {
			out.token_type = reader.string();
		} else if (reader.match("{id_token")) {
			out.id_token = reader.string();
		}
	}
	return out;
}

GoogleAccessToken test_google_access_token2()
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
	jstream::Reader reader(input);
	rule["{access_token"] = &out.access_token;
	rule["{expires_in"] = &out.expires_in;
	rule["{scope"] = &out.scope;
	rule["{token_type"] = &out.token_type;
	rule["{id_token"] = &out.id_token;
	reader.parse(rule);

	return out;
}

void main2()
{
	setlocale(LC_ALL, "fr-FR");
	char const *json = R"---(
{ "num": 123456.789 }
)---";
	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{num")) {
			printf("%f\n", r.number());
		}
	}

	jstream::Writer w([](char const *p, int n) {
		fwrite(p, 1, n, stdout);
	});
	w.object({}, [&]() {
		w.number("num", 123456.789);
	});
}

void unexpected_deep_nesting_test()
{
	char const *json = R"---(
	{
		"unexpected": {
			"values": [
				"deep",
				{
					"nesting": {
						"level": 6
					}
				},
				"done"
			]
		}
	}
)---";

	std::vector<std::string> v;

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{unexpected{values[*")) {
			if (r.isvalue()) {
				v.push_back(r.string());
			} else if (r.is_start_object()) {
				r.nest();
				do {
					if (r.match("{unexpected{values[{nesting{level")) {
						v.push_back(r.string());
					}
				} while (r.next());
			}
		}
	}

	assert(v.size() == 3);
	assert(v[0] == "deep");
	assert(v[1] == "6");
	assert(v[2] == "done");
}

int main()
{
#if 0
	test_all(true);
#elif 1
	unexpected_deep_nesting_test();
#elif 0
	test_mcp();
#else
	auto t = test_google_access_token();
	std::cout << t;
#endif

	return 0;
}
