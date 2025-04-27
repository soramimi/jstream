
#include "jstream.h"
#include "test.h"
#include <stdio.h>
#include <map>
#include <iostream>
#include <assert.h>

using namespace jstream;



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


void test()
{
	char const *json = R"---(
{
   "book":[
	  {
		 "id":"444",
		 "language":"C",
		 "edition":"First",
		 "author":"Dennis Ritchie"
	  },
	  {
		 "id":"555",
		 "language":"C++",
		 "edition":"Second",
		 "author":"Bjarne Stroustrup"
	  }
   ]
}
)---";

	std::vector<Event> expect = {
		{StartObject,    "",                        "",                        "{"},
		{Key,            "book",                    "",                        "{book"},
		{StartArray,     "book",                    "",                        "{book["},
		{StartObject,    "",                        "",                        "{book[{"},
		{Key,            "id",                      "",                        "{book[{id"},
		{String,         "",                        "444",                     "{book[{id"},
		{Key,            "language",                "",                        "{book[{language"},
		{String,         "",                        "C",                       "{book[{language"},
		{Key,            "edition",                 "",                        "{book[{edition"},
		{String,         "",                        "First",                   "{book[{edition"},
		{Key,            "author",                  "",                        "{book[{author"},
		{String,         "",                        "Dennis Ritchie",          "{book[{author"},
		{EndObject,      "",                        "",                        "{book["},
		{StartObject,    "",                        "",                        "{book[{"},
		{Key,            "id",                      "",                        "{book[{id"},
		{String,         "",                        "555",                     "{book[{id"},
		{Key,            "language",                "",                        "{book[{language"},
		{String,         "",                        "C++",                     "{book[{language"},
		{Key,            "edition",                 "",                        "{book[{edition"},
		{String,         "",                        "Second",                  "{book[{edition"},
		{Key,            "author",                  "",                        "{book[{author"},
		{String,         "",                        "Bjarne Stroustrup",       "{book[{author"},
		{EndObject,      "",                        "",                        "{book["},
		{EndArray,       "book",                    "",                        "{book"},
		{EndObject,      "",                        "",                        ""},
	};

	std::vector<Event> actual = parse_to_events(json);

	assert(actual.size() == expect.size());
	for (size_t i = 0; i < actual.size(); i++) {
		assert(actual[i] == expect[i]);
	}

}

int main()
{
#if 0
	test_all(true);
#elif 1
	test();
#else
	auto t = test_google_access_token();
	std::cout << t;
#endif

	return 0;
}
