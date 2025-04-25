
#include "../test.h"
#include <gtest/gtest.h>
#include <variant>

//

using namespace jstream;


TEST(Json, Json0)
{
	Variant var;
	Object::VariantRef ref(Array());

	Variant v;
	v = null;
	EXPECT_TRUE(is_null(v));
	v = true;
	EXPECT_EQ(get<bool>(v), true);
	v = false;
	EXPECT_EQ(get<bool>(v), false);
	v = 3.14;
	EXPECT_EQ(get<double>(v), 3.14);
	v = "Hello";
	EXPECT_EQ(get<std::string>(v), "Hello");
}

TEST(Json, Json1)
{
	char const *json = R"---(
{
	"name": "John",
	"age": 30,
	"city": "New York"
}
)---";
	jstream::Reader reader(json);
	while (reader.next()) {
		if (reader.match("{name")) {
			EXPECT_EQ(reader.string(), "John");
		} else if (reader.match("{age")) {
			EXPECT_EQ(reader.number(), 30);
		} else if (reader.match("{city")) {
			EXPECT_EQ(reader.string(), "New York");
		}
	}
}

TEST(Json, Json2)
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
}
)---";
	struct ParsedData {
		Variant name;
		Variant age;
		Variant city;
		struct Address {
			Variant street;
			Variant zip;
		} address;
	} parsed;

	jstream::Reader reader(json);
	while (reader.next()) {
		if (reader.match("{name")) {
			parsed.name = reader.string();
		} else if (reader.match("{age")) {
			parsed.age = reader.number();
		} else if (reader.match("{city")) {
			parsed.city = reader.string();
		} else if (reader.match("{address{street")) {
			parsed.address.street = reader.string();
		} else if (reader.match("{address{zip")) {
			parsed.address.zip = reader.string();
		}
	}
	EXPECT_EQ(get<std::string>(parsed.name), "John");
	EXPECT_EQ(get<double>(parsed.age), 30);
	EXPECT_EQ(get<std::string>(parsed.city), "New York");
	EXPECT_EQ(get<std::string>(parsed.address.street), "123 Main St");
	EXPECT_EQ(get<std::string>(parsed.address.zip), "10001");
}

TEST(Json, Json3)
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
}
)---";
	struct ParsedData {
		Variant name;
		Variant age;
		Variant city;
		Variant address;
	} parsed;

	jstream::Reader reader(json);
	while (reader.next()) {
		if (reader.match("{name")) {
			parsed.name = reader.string();
		} else if (reader.match("{age")) {
			parsed.age = reader.number();
		} else if (reader.match("{city")) {
			parsed.city = reader.string();
		} else if (reader.match("{address{street")) {
			obj(parsed.address)["street"] = reader.string();
		} else if (reader.match("{address{zip")) {
			obj(parsed.address)["zip"] = reader.string();
		}
	}
	EXPECT_EQ(get<std::string>(parsed.name), "John");
	EXPECT_EQ(get<double>(parsed.age), 30);
	EXPECT_EQ(get<std::string>(parsed.city), "New York");
	EXPECT_EQ(get<std::string>(Object(parsed.address).value("street")), "123 Main St");
	EXPECT_EQ(get<std::string>(Object(parsed.address).value("zip")), "10001");
}

TEST(Json, Json4)
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
	struct ParsedData {
		std::string name;
		double age;
		std::string city;
		Variant address;
		Variant hobby;
	} parsed;

	jstream::Reader reader(json);
	while (reader.next()) {
		if (reader.match("{name")) {
			parsed.name = reader.string();
		} else if (reader.match("{age")) {
			parsed.age = reader.number();
		} else if (reader.match("{city")) {
			parsed.city = reader.string();
		} else if (reader.match("{address{street")) {
			obj(parsed.address)["street"] = reader.string();
		} else if (reader.match("{address{zip")) {
			obj(parsed.address)["zip"] = reader.string();
		} else if (reader.match("{hobby[*") && reader.isstring()) {
			arr(parsed.hobby) += reader.string();
		}
	}
	EXPECT_EQ(parsed.name, "John");
	EXPECT_EQ(parsed.age, 30);
	EXPECT_EQ(parsed.city, "New York");
	EXPECT_EQ(obj(parsed.address).get<std::string>("street"), "123 Main St");
	EXPECT_EQ(obj(parsed.address).get<std::string>("zip"), "10001");
	assert(arr(parsed.hobby).size() == 5);
	EXPECT_EQ(arr(parsed.hobby).get<std::string>(0), "reading");
	EXPECT_EQ(arr(parsed.hobby).get<std::string>(1), "traveling");
	EXPECT_EQ(arr(parsed.hobby).get<std::string>(2), "coding");
	EXPECT_EQ(arr(parsed.hobby).get<std::string>(3), "cooking");
	EXPECT_EQ(arr(parsed.hobby).get<std::string>(4), "gaming");
}

TEST(Json, Json5)
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
		"traveling"
	]
}
)---";
	struct ParsedData {
		Variant name;
		Variant age;
		Variant city;
		Variant address;
		Array hobby;
	} parsed;

	jstream::Reader reader(json);
	while (reader.next()) {
		if (reader.match("{name")) {
			parsed.name = reader.string();
		} else if (reader.match("{age")) {
			parsed.age = reader.number();
		} else if (reader.match("{city")) {
			parsed.city = reader.string();
		} else if (reader.match("{address{street")) {
			obj(parsed.address)["street"] = reader.string();
		} else if (reader.match("{address{zip")) {
			obj(parsed.address)["zip"] = reader.string();
		} else if (reader.match("{hobby[*") && reader.isstring()) {
			parsed.hobby += reader.string();
		}
	}
	EXPECT_EQ(get<std::string>(parsed.name), "John");
	EXPECT_EQ(get<double>(parsed.age), 30);
	EXPECT_EQ(get<std::string>(parsed.city), "New York");
	EXPECT_EQ(obj(parsed.address).get<std::string>("street"), "123 Main St");
	EXPECT_EQ(obj(parsed.address).get<std::string>("zip"), "10001");
	assert(parsed.hobby.size() == 2);
	EXPECT_EQ(parsed.hobby.get<std::string>(0), "reading");
	EXPECT_EQ(parsed.hobby.get<std::string>(1), "traveling");

	// alternative array access
	EXPECT_EQ(get<std::string>(parsed.hobby[0]), "reading");
	EXPECT_EQ(get<std::string>(parsed.hobby[1]), "traveling");
}

TEST(Json, Json6)
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

	EXPECT_EQ(v.size(), 3);
	EXPECT_EQ(v[0], "deep");
	EXPECT_EQ(v[1], "6");
	EXPECT_EQ(v[2], "done");
}

TEST(Json, Array1)
{
	char const *json = R"---(
{
	"a": [ 12, 34, 56, 78 ]
}
)---";

	std::vector<double> v;

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{a[*")) {
			v.push_back(r.number());
		}
	}
	assert(v.size() == 4);
	EXPECT_EQ(v[0], 12);
	EXPECT_EQ(v[1], 34);
	EXPECT_EQ(v[2], 56);
	EXPECT_EQ(v[2], 56);
	EXPECT_EQ(v[3], 78);
}

TEST(Json, Array2)
{
	char const *json = R"---(
{
	"a": [[ 12, 34 ],[ 56, 78 ]]
}
)---";

	std::vector<double> v;

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{a[**")) {
			if (r.is_start_array()) {
				r.nest();
				do {
					if (r.match("{a[[*") && r.isnumber()) {
						v.push_back(r.number());
					}
				} while (r.next());
			}
		}
	}
	assert(v.size() == 4);
	EXPECT_EQ(v[0], 12);
	EXPECT_EQ(v[1], 34);
	EXPECT_EQ(v[2], 56);
	EXPECT_EQ(v[3], 78);
}

TEST(Json, Array3)
{
	char const *json = R"---(
[
	{ "name": "apple", "price": 150 },
	{ "name": "banana", "price": 80 },
	{ "name": "orange", "price": 52 }
]
)---";

	struct Item {
		std::string name;
		double price;
	};

	std::vector<Item> items;

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("[**")) {
			if (r.is_start_object()) {
				Item item;
				r.nest();
				do {
					if (r.match("[{name")) {
						item.name = r.string();
					} else if (r.match("[{price")) {
						item.price = r.number();
					}
				} while (r.next());
				items.push_back(item);
			}
		}
	}

	assert(items.size() == 3);
	EXPECT_EQ(items[0].name, "apple");
	EXPECT_EQ(items[0].price, 150);
	EXPECT_EQ(items[1].name, "banana");
	EXPECT_EQ(items[1].price, 80);
	EXPECT_EQ(items[2].name, "orange");
	EXPECT_EQ(items[2].price, 52);}

TEST(Json, Array4)
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

	struct Book {
		std::string id;
		std::string language;
		std::string edition;
		std::string author;
	};
	std::vector<Book> books;

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{book[*")) {
			Book book;
			r.nest();
			do {
				if (r.match("{book[{id")) {
					book.id = r.string();
				} else if (r.match("{book[{language")) {
					book.language = r.string();
				} else if (r.match("{book[{edition")) {
					book.edition = r.string();
				} else if (r.match("{book[{author")) {
					book.author = r.string();
				}
			} while (r.next());
			books.push_back(book);
		}
	}

	assert(books.size() == 2);
	EXPECT_EQ(books[0].id, "444");
	EXPECT_EQ(books[0].language, "C");
	EXPECT_EQ(books[0].edition, "First");
	EXPECT_EQ(books[0].author, "Dennis Ritchie");
	EXPECT_EQ(books[1].id, "555");
	EXPECT_EQ(books[1].language, "C++");
	EXPECT_EQ(books[1].edition, "Second");
	EXPECT_EQ(books[1].author, "Bjarne Stroustrup");
}

TEST(Json, GoogleForms1)
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

	struct ParsedData {
		struct Response {
			std::string resourceId;
			std::string createTime;
			std::string lastSubmittedTime;
			struct Answers {
				std::string key;
				std::string questionId;
				struct TextAnswers {
					struct Answer {
						std::string value;
					};
					std::vector<Answer> answers;
				} textAnswers;
			};
			std::vector<Answers> answers;
		};
		std::vector<Response> responses;
	} parsed;

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{responses[*") && r.is_start_object()) {
			ParsedData::Response response;
			r.nest();
			do {
				if (r.match("{responses[{responseId")) {
					response.resourceId = r.string();
				} else if (r.match("{responses[{createTime")) {
					response.createTime = r.string();
				} else if (r.match("{responses[{lastSubmittedTime")) {
					response.lastSubmittedTime = r.string();
				} else if (r.match("{responses[{answers{*")) {
					if (r.is_start_object()) {
						r.nest();
						ParsedData::Response::Answers answers;
						answers.key = r.key();
						do {
							if (r.match("{responses[{answers{*{questionId")) {
								answers.questionId = r.string();
							} else if (r.match("{responses[{answers{*{textAnswers{answers[{value")) {
								ParsedData::Response::Answers::TextAnswers::Answer a;
								a.value = r.string();
								answers.textAnswers.answers.push_back(a);
							}
						} while (r.next());
						response.answers.push_back(answers);
					}
				}
			} while (r.next());
			parsed.responses.push_back(response);
		}
	}

	assert(parsed.responses.size() == 1);
	EXPECT_EQ(parsed.responses[0].resourceId, "ACYDBNiWZb5bf6PCC964lJBNSaQfEjfc4rCTBMaAjb62EhOwJMqXCxS9NreBtx7DxOWlpRA");
	assert(parsed.responses[0].answers.size() == 3);
	EXPECT_EQ(parsed.responses[0].answers[0].key, "704f9a53");
	EXPECT_EQ(parsed.responses[0].answers[0].questionId, "704f9a53");
	EXPECT_EQ(parsed.responses[0].answers[0].textAnswers.answers[0].value, "soramimi");
	EXPECT_EQ(parsed.responses[0].answers[1].key, "4842f816");
	EXPECT_EQ(parsed.responses[0].answers[1].questionId, "4842f816");
	EXPECT_EQ(parsed.responses[0].answers[1].textAnswers.answers[0].value, "L");
	EXPECT_EQ(parsed.responses[0].answers[2].key, "1e9dfb54");
	EXPECT_EQ(parsed.responses[0].answers[2].questionId, "1e9dfb54");
	EXPECT_EQ(parsed.responses[0].answers[2].textAnswers.answers[0].value, "test");


}

TEST(Json, OAuth1)
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

	struct ParsedData {
		std::string access_token;
		double expires_in;
		std::string scope;
		std::string token_type;
		std::string id_token;
	} parsed;

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{access_token")) {
			parsed.access_token = r.string();
		} else if (r.match("{expires_in")) {
			parsed.expires_in = r.number();
		} else if (r.match("{scope")) {
			parsed.scope = r.string();
		} else if (r.match("{token_type")) {
			parsed.token_type = r.string();
		} else if (r.match("{id_token")) {
			parsed.id_token = r.string();
		}
	}

	EXPECT_EQ(parsed.access_token, "qwerty123");
	EXPECT_EQ(parsed.expires_in, 3599);
	EXPECT_EQ(parsed.scope, "https://www.googleapis.com/auth/userinfo.profile");
	EXPECT_EQ(parsed.token_type, "Bearer");
	EXPECT_EQ(parsed.id_token, "abcdefg");
}

