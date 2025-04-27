
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

int main()
{
#if 0
	test_all(true);
#elif 0
	test_mcp();
#else
	auto t = test_google_access_token();
	std::cout << t;
#endif

	return 0;
}
