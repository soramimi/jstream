#include "test.h"
#include <gtest/gtest.h>

using namespace jstream;

TEST(Json, Event1)
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

	ASSERT_EQ(actual.size(), expect.size());
	for (size_t i = 0; i < actual.size(); i++) {
		EXPECT_EQ(actual[i], expect[i]);
	}
}
