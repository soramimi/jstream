
#include "helper.h"
#include <gtest/gtest.h>

void debug1()
{
}

void debug2()
{
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
