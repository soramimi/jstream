
#include "test.h"
#include <gtest/gtest.h>

//

using namespace jstream;


TEST(Json, Json0)
{
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
	ASSERT_EQ(arr(parsed.hobby).size(), 5);
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
	ASSERT_EQ(parsed.hobby.size(), 2);
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

TEST(Json, Json7)
{
	char const *json = R"---(
{
  "floats": {
	"smallest_subnormal": 5e-324,
	"largest_subnormal": 2.2250738585072009e-308,
	"smallest_normal": 2.2250738585072014e-308,
	"largest_normal": 1.7976931348623157e+308,

	"just_over_one": 1.0000000000000002,
	"just_under_one": 0.9999999999999999,

	"negative_just_over_minus_one": -0.9999999999999999,
	"negative_just_under_minus_one": -1.0000000000000002,

	"pi_high_prec": 3.14159265358979323846264338327950288419716939937510,
	"e_high_prec": 2.71828182845904523536028747135266249775724709369995,

	"half": 0.5,
	"one_third": 0.3333333333333333,
	"two_thirds": 0.6666666666666666,

	"binary_rounding_issue_1": 0.1,
	"binary_rounding_issue_2": 0.2,
	"binary_rounding_issue_3": 0.3,
	"binary_rounding_issue_4": 0.4,

	"precision_loss": 9007199254740993,
	"int_boundary": 9007199254740991,
  }
}
)---";

	struct ParsedData {
		double smallest_subnormal;
		double largest_subnormal;
		double smallest_normal;
		double largest_normal;
		double just_over_one;
		double just_under_one;
		double negative_just_over_minus_one;
		double negative_just_under_minus_one;
		double pi_high_prec;
		double e_high_prec;
		double half;
		double one_third;
		double two_thirds;
		double binary_rounding_issue_1;
		double binary_rounding_issue_2;
		double binary_rounding_issue_3;
		double binary_rounding_issue_4;
		double precision_loss;
		double int_boundary;
	} parsed;

	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{floats{smallest_subnormal")) {
			parsed.smallest_subnormal = r.number();
		} else if (r.match("{floats{largest_subnormal")) {
			parsed.largest_subnormal = r.number();
		} else if (r.match("{floats{smallest_normal")) {
			parsed.smallest_normal = r.number();
		} else if (r.match("{floats{largest_normal")) {
			parsed.largest_normal = r.number();
		} else if (r.match("{floats{just_over_one")) {
			parsed.just_over_one = r.number();
		} else if (r.match("{floats{just_under_one")) {
			parsed.just_under_one = r.number();
		} else if (r.match("{floats{negative_just_over_minus_one")) {
			parsed.negative_just_over_minus_one = r.number();
		} else if (r.match("{floats{negative_just_under_minus_one")) {
			parsed.negative_just_under_minus_one = r.number();
		} else if (r.match("{floats{pi_high_prec")) {
			parsed.pi_high_prec = r.number();
		} else if (r.match("{floats{e_high_prec")) {
			parsed.e_high_prec = r.number();
		} else if (r.match("{floats{half")) {
			parsed.half = r.number();
		} else if (r.match("{floats{one_third")) {
			parsed.one_third = r.number();
		} else if (r.match("{floats{two_thirds")) {
			parsed.two_thirds = r.number();
		} else if (r.match("{floats{binary_rounding_issue_1")) {
			parsed.binary_rounding_issue_1 = r.number();
		} else if (r.match("{floats{binary_rounding_issue_2")) {
			parsed.binary_rounding_issue_2 = r.number();
		} else if (r.match("{floats{binary_rounding_issue_3")) {
			parsed.binary_rounding_issue_3 = r.number();
		} else if (r.match("{floats{binary_rounding_issue_4")) {
			parsed.binary_rounding_issue_4 = r.number();
		} else if (r.match("{floats{precision_loss")) {
			parsed.precision_loss = r.number();
		} else if (r.match("{floats{int_boundary")) {
			parsed.int_boundary = r.number();
		}
	}

	double eps = 1e-15;

	EXPECT_NEAR(parsed.smallest_subnormal, 5e-324, eps);
	EXPECT_NEAR(parsed.largest_subnormal, 2.2250738585072009e-308, eps);
	EXPECT_NEAR(parsed.smallest_normal, 2.2250738585072014e-308, eps);
	EXPECT_NEAR(parsed.largest_normal, 1.7976931348623157e+308, eps);
	EXPECT_NEAR(parsed.just_over_one, 1.0000000000000002, eps);
	EXPECT_NEAR(parsed.just_under_one, 0.9999999999999999, eps);
	EXPECT_NEAR(parsed.negative_just_over_minus_one, -0.9999999999999999, eps);
	EXPECT_NEAR(parsed.negative_just_over_minus_one, -1.0000000000000002, eps);
	EXPECT_NEAR(parsed.pi_high_prec, 3.14159265358979323846264338327950288419716939937510, eps);
	EXPECT_NEAR(parsed.pi_high_prec, 3.14159265358979323846264338327950288419716939937510, eps);
	EXPECT_NEAR(parsed.e_high_prec, 2.71828182845904523536028747135266249775724709369995, eps);
	EXPECT_NEAR(parsed.half, 0.5, eps);
	EXPECT_NEAR(parsed.one_third, 0.3333333333333333, eps);
	EXPECT_NEAR(parsed.two_thirds, 0.6666666666666666, eps);
	EXPECT_NEAR(parsed.binary_rounding_issue_1, 0.1, eps);
	EXPECT_NEAR(parsed.binary_rounding_issue_2, 0.2, eps);
	EXPECT_NEAR(parsed.binary_rounding_issue_3, 0.3, eps);
	EXPECT_NEAR(parsed.binary_rounding_issue_4, 0.4, eps);
	EXPECT_NEAR(parsed.precision_loss, 9007199254740993, eps);
	EXPECT_NEAR(parsed.int_boundary, 9007199254740991, eps);
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
	ASSERT_EQ(v.size(), 4);
	EXPECT_EQ(v[0], 12);
	EXPECT_EQ(v[1], 34);
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
		if (r.match_start_array("{a[**")) {
			r.nest();
			do {
				if (r.match("{a[[*") && r.isnumber()) {
					v.push_back(r.number());
				}
			} while (r.next());
		}
	}
	ASSERT_EQ(v.size(), 4);
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
		if (r.match_start_object("[**")) {
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

	ASSERT_EQ(items.size(), 3);
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

	ASSERT_EQ(books.size(), 2);
	EXPECT_EQ(books[0].id, "444");
	EXPECT_EQ(books[0].language, "C");
	EXPECT_EQ(books[0].edition, "First");
	EXPECT_EQ(books[0].author, "Dennis Ritchie");
	EXPECT_EQ(books[1].id, "555");
	EXPECT_EQ(books[1].language, "C++");
	EXPECT_EQ(books[1].edition, "Second");
	EXPECT_EQ(books[1].author, "Bjarne Stroustrup");
}

