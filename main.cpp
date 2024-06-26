
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

void main2()
{
	std::string input = R"---(
{
	"hoge": {
		"foo": 123,
		"bar": 456
	}
}
)---";
	std::vector<double> vals;
	jstream::Reader json(input.c_str(), input.size());
	while (json.next()) {
		printf("%d: %s\n", json.state(), json.path().c_str());
		if (json.match_end_object("{hoge")) {
			puts("end hoge!");
		}
	}
}

int main()
{
#if 1
	test_all();
#elif 1
	main2();
#else
	auto t = parse();
	print(t);
#endif

	return 0;
}
