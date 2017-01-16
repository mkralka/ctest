#include <ctest/tests.h>

#include "romnum.h"

CT_DATA_TYPE(valid_input) {
	const char *rn;
	int value;
};

CT_DATA(valid_input) {
	{ "i", 1 },
	{ "ii", 2 },
	{ "iii", 3 },
	{ "iv", 4 },
	{ "v", 5 },
	{ "vi", 6 },
	{ "vii", 7 },
	{ "viii", 8 },
	{ "ix", 9 },
	{ "x", 10 },
	{ "xi", 11 },
	{ "xiv", 14 },
	{ "xix", 19 },
	{ "xxxix", 39 },
	{ "xl", 40 },
	{ "xliv", 44 },
	{ "l", 50 },
	{ "lix", 59 },
	{ "lx", 60 },
	{ "xc", 90 },
	{ "xciv", 94 },
	{ "c", 100 },
	{ "cxliv", 144 },
	{ "cl", 150 },
	{ "cm", 900 },
	{ "m", 1000 },
	{ "mcmlxx", 1970 },
};

CT_DATA_PROVIDER(valid_input, "{rn=\"%s\", value=%d}", data->rn, data->value);

CT_TEST_WITH_DATA(valid_input, valid_input)
{
	CT_ASSERT_INT_EQ(rntoi(data->rn), data->value);
}

CT_SUITE_TESTS(romnum) {
	CT_SUITE_TEST(valid_input),
};
CT_SUITE(romnum);
