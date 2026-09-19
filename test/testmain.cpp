
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
	"array": {
		{
			"value": 123
		}
	}
}
)---";
		
	std::vector<std::string> v;
	
	jstream::Reader r(json);
	while (r.next()) {
		if (r.match("{array{{*")) {
			if (r.is_constant()) {
				v.push_back(r.string());
			}
		}
	}
}

int main(int argc, char **argv)
{
	if (1) {
		debug1();
	}
	
	::testing::InitGoogleTest(&argc, argv);
	int ret = RUN_ALL_TESTS();

	return ret;
}
