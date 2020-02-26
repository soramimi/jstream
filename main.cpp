
#include "jstream.h"
#include "test.h"
#include <stdio.h>

struct GoogleAccessToken {
	std::string access_token;
	std::string token_type;
	std::string expires_in;
	std::string scope;
	std::string refresh_token;
	std::string id_token;
};

GoogleAccessToken parse()
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

void print(GoogleAccessToken const &token)
{
	jstream::Writer w([](char const *p, int n){
		fwrite(p, 1, n, stdout);
	});

	w.object({}, [&](){
		w.string(token.access_token, "access_token");
		w.string(token.expires_in, "expires_in");
		w.string(token.scope, "scope");
		w.string(token.token_type, "token_type");
		w.string(token.id_token, "id_token");
	});
}

int main()
{
#if 1
	auto t = parse();
	print(t);
#else
	test_all();
#endif

	return 0;
}
