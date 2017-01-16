# Upgrading Your Tests with Data

Some unit tests are better represented as boilerplates that can be run on
different input data. Consider the `rntoi` function introduced
[earlier](tutorial_first_suite.md). Positive test cases are essentially a list
of Roman numeral/integer pairs that are equivalent; parsing the Roman numeral
should result in corresponding integer and formatting the integer as a Romain
numeral should generate the corresponding Romain numeral.

A naïve approach to reducing test code might involve defining a list of input
data that is iterated over in your test. The result might look similar to the
following:

```c
CT_TEST(valid_input)
{
	static const struct { const char *numeral; int value; } input[] = {
		{ "i", 1 },
		{ "v", 5 },
		{ "x", 10 },
		{ "l", 50 },
		{ "c", 100 },
		{ "m", 1000 },
	};
	size_t i;

	for (i = 0; i < sizeof(input)/sizeof(*input); ++i) {
		CT_ASSERT_INT_EQ(rntoi(input[i].numeral), input[i].value);
	}
}
```

(Granted, this isn't a particularly compelling use-case for iterating over input
data since the body of the loop is a single assertion.)

Although this gets the job done, it doesn't integrate well with the testing
framework. If the test fails for any input, the subsequent input will be
skipped. This can hide useful information about the overall behavior of the
code under test. Knowing all the cases that fail may point to where in the code
that things are broken.

A better way to handle this is to use a data provider. A data provider is a
collection of data that can be passed to a test function and recorded
independently.

## Defining a Data Provider

The data provider that replaces the above example would look very similar to the
following:

```c
CT_DATA_TYPE(valid_input) {
	const char *rn;
	int value;
};

CT_DATA(valid_input) {
	{ "i", 1 },
	{ "v", 5 },
	{ "x", 10 },
	{ "l", 50 },
	{ "c", 100 },
	{ "m", 1000 },
};

CT_DATA_PROVIDER(valid_input, "{rn=\"%s\", value=%d}", data->rn, data->value);
```

As you can see, a data provider is split into threw components: the type
definition, the data to provide, and the provider itself.

The `CT_DATA_TYPE` macro is used to define the type associated with the
provider. This is essentially a `struct` with one or more fields needed by the
test; think of these as the parameters to the test. Use of `CT_DATA_TYPE` macro
looks a lot like defining a `struct`. The parameter to `CT_DATA_TYPE` (in this
case, `valid_input`) is the name of the data provider for which this type is
being defined; it must match the name specified by the other components of this
provider.

The `CT_DATA` macro is used to define the data to be provided. This is
essentially an array of the `struct`s defined by the associated `CT_DATA_TYPE`.
Use of `CT_DATA` macro looks a lot like initializing an array. The parameter to
`CT_DATA` (in this case, `valid_input`) is the name of the data provider for
which this data is being defined; it must match the name specified by the other
components of this provider.

Finally, the `CT_DATA_PROVIDER` macro is used to pull together the type and
data into a provider. The first parameter to `CT_DATA_PROVIDER` (in this case,
`valid_input`) is the name of the provider; this must match the name specified
by the other components of the provider and can be used to reference this
provider from test definitions. The remaining parameters describe `printf`-style
string and related parameters for creating a human-readable string for the
parameters. Fields from the provider's `struct` are referenced relative to a
pointer named `data`. Note that the human readable string should be short and be
contained on a single line; this string will be used when creating the name of
the test case associated with the provided data.

## Using Data Provider with a Test

A test that is powered by a data provider looks very much like any other test
except that it is defined using the `CT_TEST_WITH_DATA` macro.
`CT_TEST_WITH_DATA` is similar to `CT_TEST` except it takes an additional
parameter indicating the data provider that will provide data for the test case.
The provided data can be references from within the body of the test through the
`data` variable.

Redefining the test from [earlier](tutoral_first_suite.md), our data provider
test might look similar to the following:

```c
CT_TEST_WITH_DATA(valid_input, valid_input)
{
	CT_ASSERT_INT_EQ(rntoi(data->rn), data->value);
}
```

Notice how both the first and second parameters are `valid_input`? This is a
coincidence. The first `valid_input` refers to the name of the test while the
second `valid_input` refers to the name of the data provider (much like you can
have a `struct` and an `enum` with the same name, it's possible to have a test
and data provider with the same name).

Also notice that `valid_input` matches the name of of the data provider defined
above with `CT_DATA_PROVIDER`. If the name of the data provider doesn't match
one that was defined, a compiler error will be generated.

Data-provider-enabled tests are registered using the `CT_SUITE_TEST` macro in
the same way as other tests:

```c
CT_SUITE_TESTS(romnum) {
	CT_SUITE_TEST(valid_input),
};
CT_SUITE(romnum);
```

## Pulling it all Together

If we combine this all together, the complete test suite may look similar to the
following, if a few more test cases were added:

```c
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
```

(This test file can be found in
[`src/examples/suite_with_data.c`](../../src/examples/suite_with_data.c)).

## Running Tests with Data Providers

Running tests that are supplied data by providers is no different than
non-provider test cases. However, the output is different. Each element from the
provider is converted into a string (using the supplied method) and is then
appended to the end of the name of the test. This makes it easy to see which
test cases are succeeding or failing.

You can run the compiled examples as follows:

```
$ src/cli/ctester run src/examples/suite_with_data.la
romnum:valid_input[{rn="i", value=1}] ... OK
romnum:valid_input[{rn="ii", value=2}] ... OK
romnum:valid_input[{rn="iii", value=3}] ... OK
romnum:valid_input[{rn="iv", value=4}] ... OK
romnum:valid_input[{rn="v", value=5}] ... OK
romnum:valid_input[{rn="vi", value=6}] ... OK
romnum:valid_input[{rn="vii", value=7}] ... OK
romnum:valid_input[{rn="viii", value=8}] ... OK
romnum:valid_input[{rn="ix", value=9}] ... OK
romnum:valid_input[{rn="x", value=10}] ... OK
romnum:valid_input[{rn="xi", value=11}] ... OK
romnum:valid_input[{rn="xiv", value=14}] ... OK
romnum:valid_input[{rn="xix", value=19}] ... OK
romnum:valid_input[{rn="xxxix", value=39}] ... OK
romnum:valid_input[{rn="xl", value=40}] ... OK
romnum:valid_input[{rn="xliv", value=44}] ... OK
romnum:valid_input[{rn="l", value=50}] ... OK
romnum:valid_input[{rn="lix", value=59}] ... OK
romnum:valid_input[{rn="lx", value=60}] ... OK
romnum:valid_input[{rn="xc", value=90}] ... OK
romnum:valid_input[{rn="xciv", value=94}] ... OK
romnum:valid_input[{rn="c", value=100}] ... OK
romnum:valid_input[{rn="cxliv", value=144}] ... OK
romnum:valid_input[{rn="cl", value=150}] ... OK
romnum:valid_input[{rn="cm", value=900}] ... OK
romnum:valid_input[{rn="m", value=1000}] ... OK
romnum:valid_input[{rn="mcmlxx", value=1970}] ... OK
```

---
Next: [Upgrading Your Tests with Fixtures](tutorial_fixtures.md)
