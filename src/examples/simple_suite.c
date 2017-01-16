#include <ctest/tests.h>

#include "romnum.h"

CT_TEST(valid_input)
{
	CT_ASSERT_INT_EQ(rntoi("i"), 1);
	CT_ASSERT_INT_EQ(rntoi("v"), 5);
	CT_ASSERT_INT_EQ(rntoi("x"), 10);
	CT_ASSERT_INT_EQ(rntoi("l"), 50);
	CT_ASSERT_INT_EQ(rntoi("c"), 100);
	CT_ASSERT_INT_EQ(rntoi("m"), 1000);
}

CT_SUITE_TESTS(romnum) {
	CT_SUITE_TEST(valid_input),
};
CT_SUITE(romnum);
