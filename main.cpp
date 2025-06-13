
#include <jstream.h>
#include "test/test.h"
#include <stdio.h>
#include <assert.h>

using namespace jstream;


void main1()
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

	struct ParsedData {
		std::string id;
		std::string language;
		std::string edition;
		std::string author;
	};
	std::vector<ParsedData> books;

	jstream::Reader reader(json);
	while (reader.next()) {
		if (reader.match_start_object("{book[**")) {
			reader.nest();
			ParsedData book;
			do {
				if (reader.match("{book[{id")) {
					book.id = reader.string();
				} else if (reader.match("{book[{language")) {
					book.language = reader.string();
				} else if (reader.match("{book[{edition")) {
					book.edition = reader.string();
				} else if (reader.match("{book[{author")) {
					book.author = reader.string();
				}
			} while (reader.next());
			books.push_back(book);
		}
	}
	printf("%d\n", books.size());
}


void main2()
{
#if 0
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
#endif
}


void print()
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

	print_events(json);
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

void test2()
{
	jstream::Writer w([](char const *p, int n) {
		fwrite(p, 1, n, stdout);
	});

	w.object({}, [&]() {
		w.string("name", "John Doe");
		w.string("age", "30");
		w.string("city", "New York");
	});

}

int main()
{
#if 0
	test_all(true);
#elif 0
	print();
#else
	main1();
#endif

	return 0;
}
