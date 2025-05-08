
#include "test.h"
#include <gtest/gtest.h>

//

using namespace jstream;


TEST(Json, JsonBasic0)
{
	const char *json = R"---(
[
  {
	"name": "John Smith",
	"age": 34,
	"address": "Brooklyn, New York"
  },
  {
	"name": "Emily Johnson",
	"age": 28,
	"address": "Downtown, Los Angeles"
  },
  {
	"name": "Michael Brown",
	"age": 41,
	"address": "Capitol Hill, Washington D.C."
  }
]
)---";

	struct Person {
		std::string name;
		int age;
		std::string address;
	};

	std::vector<Person> persons1;
	jstream::Reader r(json);
	while (r.next()) {
		if (r.is_start_array()) {
			r.nest();
			do {
				if (r.is_start_object()) {
					r.nest();
					Person person;
					do {
						if (r.match("[{name")) {
							person.name = r.string();
						} else if (r.match("[{age")) {
							person.age = r.number();
						} else if (r.match("[{address")) {
							person.address = r.string();
						}
					} while (r.next());
					persons1.push_back(person);
				}
			} while (r.next());
		}
	}

	auto Check = [](std::vector<Person> const &persons){
		ASSERT_EQ(persons.size(), 3);
		EXPECT_EQ(persons[0].name, "John Smith");
		EXPECT_EQ(persons[0].age, 34);
		EXPECT_EQ(persons[0].address, "Brooklyn, New York");
		EXPECT_EQ(persons[1].name, "Emily Johnson");
		EXPECT_EQ(persons[1].age, 28);
		EXPECT_EQ(persons[1].address, "Downtown, Los Angeles");
		EXPECT_EQ(persons[2].name, "Michael Brown");
		EXPECT_EQ(persons[2].age, 41);
		EXPECT_EQ(persons[2].address, "Capitol Hill, Washington D.C.");
	};

	// parse original json to persons1
	Check(persons1);

	// re-write json from persons1 to json2

	std::vector<char> buffer;
	jstream::Writer w([&](char const *p, int n){
		buffer.insert(buffer.end(), p, p + n);
	});
	w.array({}, [&](){
		for (Person const &person : persons1) {
			w.object({}, [&](){
				w.string("name", person.name);
				w.number("age", person.age);
				w.string("address", person.address);
			});
		}
	});
	std::string json2 = std::string(buffer.data(), buffer.size());

	// parse json2 to persons2
	std::vector<Person> persons2;
	jstream::Reader r2(json);
	while (r2.next()) {
		if (r2.is_start_array()) {
			r2.nest();
			do {
				if (r2.is_start_object()) {
					r2.nest();
					Person person;
					do {
						if (r2.match("[{name")) {
							person.name = r2.string();
						} else if (r2.match("[{age")) {
							person.age = r2.number();
						} else if (r2.match("[{address")) {
							person.address = r2.string();
						}
					} while (r2.next());
					persons2.push_back(person);
				}
			} while (r2.next());
		}
	}

	Check(persons2);
}
