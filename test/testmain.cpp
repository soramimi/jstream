
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

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
