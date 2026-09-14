
#include "test.h"
#include <gtest/gtest.h>



void test_parse_event(char const *json, std::vector<Event> const &expect)
{
	std::vector<Event> actual = parse_to_events(json);

	ASSERT_EQ(actual.size(), expect.size());
	for (size_t i = 0; i < actual.size(); i++) {
		EXPECT_EQ(actual[i], expect[i]);
	}
}

void debug1()
{
	char const *json = R"---(
{
	"name": "John",
	"age": 30,
	"city": "New York"
}
)---";
	std::string name;
	int age = 0;
	std::string city;
	jstream::Reader reader(json);
	while (reader.next()) {
		if (reader.match("{name")) {
			name = reader.string();
			fprintf(stderr, "name=%s\n", name.c_str());
		} else if (reader.match("{age")) {
			age = reader.number();
			fprintf(stderr, "age=%d\n", age);
		} else if (reader.match("{city")) {
			city = reader.string();
			fprintf(stderr, "city=%s\n", city.c_str());
		}
	}	
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();

	if (0) {
		debug1();
	}

	return ret;
}
