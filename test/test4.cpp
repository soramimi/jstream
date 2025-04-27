#include "test.h"
#include <gtest/gtest.h>

using namespace jstream;

TEST(Json, Event1)
{
   char const *json = R"---(
{
	"a": [ 12, 34, 56, 78 ]
}
)---";

   std::vector<Event> expect = {
	  {StartObject, "", "", "{"},
	  {Key, "a", "", "{a"},
	  {StartArray, "a", "", "{a["},
	  {Number, "", "12", "{a["},
	  {Number, "", "34", "{a["},
	  {Number, "", "56", "{a["},
	  {Number, "", "78", "{a["},
	  {EndArray, "a", "", "{a"},
	  {EndObject, "", "", ""},
   };

   test_parse_event(json, expect);
}

TEST(Json, Event2)
{
   char const *json = R"---(
{
	"a": [[ 12, 34 ],[ 56, 78 ]]
}
)---";

   std::vector<Event> expect = {
	  {StartObject, "", "", "{"},
	  {Key, "a", "", "{a"},
	  {StartArray, "a", "", "{a["},
	  {StartArray, "", "", "{a[["},
	  {Number, "", "12", "{a[["},
	  {Number, "", "34", "{a[["},
	  {EndArray, "", "", "{a["},
	  {StartArray, "", "", "{a[["},
	  {Number, "", "56", "{a[["},
	  {Number, "", "78", "{a[["},
	  {EndArray, "", "", "{a["},
	  {EndArray, "a", "", "{a"},
	  {EndObject, "", "", ""},
   };

   test_parse_event(json, expect);
}

TEST(Json, Event3)
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

   test_parse_event(json, expect);
}

TEST(Json, Event4)
{
   char const *json = R"---(
{
   "name": "John",
   "age": 30,
   "city": "New York",
   "address": {
	   "street": "123 Main St",
	   "zip": "10001"
   },
   "hobby": [
	   "reading",
	   "traveling",
	   "coding",
	   "cooking",
	   "gaming"
   ]
}
)---";

   std::vector<Event> expect = {
	  {StartObject, "", "", "{"},
	  {Key, "name", "", "{name"},
	  {String, "", "John", "{name"},
	  {Key, "age", "", "{age"},
	  {Number, "", "30", "{age"},
	  {Key, "city", "", "{city"},
	  {String, "", "New York", "{city"},
	  {Key, "address", "", "{address"},
	  {StartObject, "address", "", "{address{"},
	  {Key, "street", "", "{address{street"},
	  {String, "", "123 Main St", "{address{street"},
	  {Key, "zip", "", "{address{zip"},
	  {String, "", "10001", "{address{zip"},
	  {EndObject, "address", "", "{address"},
	  {Key, "hobby", "", "{hobby"},
	  {StartArray, "hobby", "", "{hobby["},
	  {String, "", "reading", "{hobby["},
	  {String, "", "traveling", "{hobby["},
	  {String, "", "coding", "{hobby["},
	  {String, "", "cooking", "{hobby["},
	  {String, "", "gaming", "{hobby["},
	  {EndArray, "hobby", "", "{hobby"},
	  {EndObject, "", "", ""},
   };

   test_parse_event(json, expect);
}

TEST(Json, Event5)
{
   char const *json = R"---(
{
  "responses": [
	{
	  "responseId": "ACYDBNiWZb5bf6PCC964lJBNSaQfEjfc4rCTBMaAjb62EhOwJMqXCxS9NreBtx7DxOWlpRA",
	  "createTime": "2024-06-19T16:05:51.394Z",
	  "lastSubmittedTime": "2024-06-19T16:05:51.394652Z",
	  "answers": {
		"704f9a53": {
		  "questionId": "704f9a53",
		  "textAnswers": {
			"answers": [
			  {
				"value": "soramimi"
			  }
			]
		  }
		},
		"4842f816": {
		  "questionId": "4842f816",
		  "textAnswers": {
			"answers": [
			  {
				"value": "L"
			  }
			]
		  }
		},
		"1e9dfb54": {
		  "questionId": "1e9dfb54",
		  "textAnswers": {
			"answers": [
			  {
				"value": "test"
			  }
			]
		  }
		}
	  }
	}
  ]
}
)---";

   std::vector<Event> expect = {
	  {StartObject, "", "", "{"},
	  {Key, "responses", "", "{responses"},
	  {StartArray, "responses", "", "{responses["},
	  {StartObject, "", "", "{responses[{"},
	  {Key, "responseId", "", "{responses[{responseId"},
	  {String, "", "ACYDBNiWZb5bf6PCC964lJBNSaQfEjfc4rCTBMaAjb62EhOwJMqXCxS9NreBtx7DxOWlpRA", "{responses[{responseId"},
	  {Key, "createTime", "", "{responses[{createTime"},
	  {String, "", "2024-06-19T16:05:51.394Z", "{responses[{createTime"},
	  {Key, "lastSubmittedTime", "", "{responses[{lastSubmittedTime"},
	  {String, "", "2024-06-19T16:05:51.394652Z", "{responses[{lastSubmittedTime"},
	  {Key, "answers", "", "{responses[{answers"},
	  {StartObject, "answers", "", "{responses[{answers{"},
	  {Key, "704f9a53", "", "{responses[{answers{704f9a53"},
	  {StartObject, "704f9a53", "", "{responses[{answers{704f9a53{"},
	  {Key, "questionId", "", "{responses[{answers{704f9a53{questionId"},
	  {String, "", "704f9a53", "{responses[{answers{704f9a53{questionId"},
	  {Key, "textAnswers", "", "{responses[{answers{704f9a53{textAnswers"},
	  {StartObject, "textAnswers", "", "{responses[{answers{704f9a53{textAnswers{"},
	  {Key, "answers", "", "{responses[{answers{704f9a53{textAnswers{answers"},
	  {StartArray, "answers", "", "{responses[{answers{704f9a53{textAnswers{answers["},
	  {StartObject, "", "", "{responses[{answers{704f9a53{textAnswers{answers[{"},
	  {Key, "value", "", "{responses[{answers{704f9a53{textAnswers{answers[{value"},
	  {String, "", "soramimi", "{responses[{answers{704f9a53{textAnswers{answers[{value"},
	  {EndObject, "", "", "{responses[{answers{704f9a53{textAnswers{answers["},
	  {EndArray, "answers", "", "{responses[{answers{704f9a53{textAnswers{answers"},
	  {EndObject, "textAnswers", "", "{responses[{answers{704f9a53{textAnswers"},
	  {EndObject, "704f9a53", "", "{responses[{answers{704f9a53"},
	  {Key, "4842f816", "", "{responses[{answers{4842f816"},
	  {StartObject, "4842f816", "", "{responses[{answers{4842f816{"},
	  {Key, "questionId", "", "{responses[{answers{4842f816{questionId"},
	  {String, "", "4842f816", "{responses[{answers{4842f816{questionId"},
	  {Key, "textAnswers", "", "{responses[{answers{4842f816{textAnswers"},
	  {StartObject, "textAnswers", "", "{responses[{answers{4842f816{textAnswers{"},
	  {Key, "answers", "", "{responses[{answers{4842f816{textAnswers{answers"},
	  {StartArray, "answers", "", "{responses[{answers{4842f816{textAnswers{answers["},
	  {StartObject, "", "", "{responses[{answers{4842f816{textAnswers{answers[{"},
	  {Key, "value", "", "{responses[{answers{4842f816{textAnswers{answers[{value"},
	  {String, "", "L", "{responses[{answers{4842f816{textAnswers{answers[{value"},
	  {EndObject, "", "", "{responses[{answers{4842f816{textAnswers{answers["},
	  {EndArray, "answers", "", "{responses[{answers{4842f816{textAnswers{answers"},
	  {EndObject, "textAnswers", "", "{responses[{answers{4842f816{textAnswers"},
	  {EndObject, "4842f816", "", "{responses[{answers{4842f816"},
	  {Key, "1e9dfb54", "", "{responses[{answers{1e9dfb54"},
	  {StartObject, "1e9dfb54", "", "{responses[{answers{1e9dfb54{"},
	  {Key, "questionId", "", "{responses[{answers{1e9dfb54{questionId"},
	  {String, "", "1e9dfb54", "{responses[{answers{1e9dfb54{questionId"},
	  {Key, "textAnswers", "", "{responses[{answers{1e9dfb54{textAnswers"},
	  {StartObject, "textAnswers", "", "{responses[{answers{1e9dfb54{textAnswers{"},
	  {Key, "answers", "", "{responses[{answers{1e9dfb54{textAnswers{answers"},
	  {StartArray, "answers", "", "{responses[{answers{1e9dfb54{textAnswers{answers["},
	  {StartObject, "", "", "{responses[{answers{1e9dfb54{textAnswers{answers[{"},
	  {Key, "value", "", "{responses[{answers{1e9dfb54{textAnswers{answers[{value"},
	  {String, "", "test", "{responses[{answers{1e9dfb54{textAnswers{answers[{value"},
	  {EndObject, "", "", "{responses[{answers{1e9dfb54{textAnswers{answers["},
	  {EndArray, "answers", "", "{responses[{answers{1e9dfb54{textAnswers{answers"},
	  {EndObject, "textAnswers", "", "{responses[{answers{1e9dfb54{textAnswers"},
	  {EndObject, "1e9dfb54", "", "{responses[{answers{1e9dfb54"},
	  {EndObject, "answers", "", "{responses[{answers"},
	  {EndObject, "", "", "{responses["},
	  {EndArray, "responses", "", "{responses"},
	  {EndObject, "", "", ""},
   };

   test_parse_event(json, expect);
}

TEST(Json, Event6)
{
   char const *json = R"---(
{
  "access_token": "qwerty123",
  "expires_in": 3599,
  "scope": "https://www.googleapis.com/auth/userinfo.profile",
  "token_type": "Bearer",
  "id_token": "abcdefg"
}
)---";

   std::vector<Event> expect = {
	  {StartObject, "", "", "{"},
	  {Key, "access_token", "", "{access_token"},
	  {String, "", "qwerty123", "{access_token"},
	  {Key, "expires_in", "", "{expires_in"},
	  {Number, "", "3599", "{expires_in"},
	  {Key, "scope", "", "{scope"},
	  {String, "", "https://www.googleapis.com/auth/userinfo.profile", "{scope"},
	  {Key, "token_type", "", "{token_type"},
	  {String, "", "Bearer", "{token_type"},
	  {Key, "id_token", "", "{id_token"},
	  {String, "", "abcdefg", "{id_token"},
	  {EndObject, "", "", ""},
   };

   test_parse_event(json, expect);
}

TEST(Json, Event7)
{
   char const *json = R"---(
{
 "name": "John Smith",
 "age": 35,
 "languages": ["English", "Japanese"]
}
)---";

   std::vector<Event> expect = {
	  {StartObject, "", "", "{"},
	  {Key, "name", "", "{name"},
	  {String, "", "John Smith", "{name"},
	  {Key, "age", "", "{age"},
	  {Number, "", "35", "{age"},
	  {Key, "languages", "", "{languages"},
	  {StartArray, "languages", "", "{languages["},
	  {String, "", "English", "{languages["},
	  {String, "", "Japanese", "{languages["},
	  {EndArray, "languages", "", "{languages"},
	  {EndObject, "", "", ""},
   };

   test_parse_event(json, expect);
}

TEST(Json, Event8)
{
   char const *json = R"---(
{
	"string1": "abc",
	"string2": "\u00a9",
	"string3": "\u3042",
	"string4": "\ud834\udd1e",
	"number": 123.456789,
	"null": null,
	"false": false,
	"true": true
}
)---";

   std::vector<Event> expect = {
	  {StartObject, "", "", "{"},
	  {Key, "string1", "", "{string1"},
	  {String, "", "abc", "{string1"},
	  {Key, "string2", "", "{string2"},
	  {String, "", "©", "{string2"},
	  {Key, "string3", "", "{string3"},
	  {String, "", "あ", "{string3"},
	  {Key, "string4", "", "{string4"},
	  {String, "", "𝄞", "{string4"},
	  {Key, "number", "", "{number"},
	  {Number, "", "123.456789", "{number"},
	  {Key, "null", "", "{null"},
	  {Null, "", "null", "{null"},
	  {Key, "false", "", "{false"},
	  {False, "", "false", "{false"},
	  {Key, "true", "", "{true"},
	  {True, "", "true", "{true"},
	  {EndObject, "", "", ""},
   };

   test_parse_event(json, expect);
}
