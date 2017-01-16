# Upgrading Your Tests with Fixtures

Occasionally, tests will require an environment to be set up prior to running
the test and automatically torn down after the test (e.g., external libraries
may need to be initialized).

`ctest` supports this functionality through the concept of a test fixture. A
fixture is a collection of variables that are set up (initialized) before each
test case is executed and torn down after each test case has completed (whether
successful or not). Since each test case has its own fixture, test cases can be
run in parallel and no state is shared between test cases.

Fixtures are often abused to create variables needed for one or more of the
tests (e.g., stubs). This has the tendency to reduce the readability of the
tests because the tests are no longer self-contained (and the reader will be
forced to try and figure out where and how these variables get initialized.
Before embarking on the task of creating a new fixture, consider alternatives.

For this exercise, we will focus on unit testing two new functions that you have
written:

```c
void hello_world()
{
	printf("Hello, World!\n");
}

void hello_person(const char *name)
{
	printf("Hello, %s!\n", name);
}
```

(These can be found in
[`src/examples/hello_world.c`](../../src/examples/hello_world.c).)

Notice how these functions write to `stdout`; this makes testing a bit tricky.
If we can redirect `stdout` to a file, then we can compare the contents of said
file to what we expect, allowing us to verify that they print what is expected.
A fixture is perfect for this.

## Defining a Fixture

A fixture is composed of three components: state `struct`, set up routine, and
tear down routine. The state `struct` defines the state of the fixture; that is,
the variables that are available to the test case. The set up routine is used to
initialize a state `struct`; this is called automatically by `ctest`.
The tear down routine is used to finalize (destroy) a state `struct`;
this is called automatically by the `ctest`, whether the test is successful or
not (it is always called if the setup returns successfully).

Let's look at fixture to help solve our `stdout` redirection problem:

```c
#define TMPFILE_TEMPLATE "/tmp/fixtures-XXXXXXXX"

CT_FIXTURE_TYPE(fixture) {
	int orig_stdout_fd;
	char tmpfile[sizeof(TMPFILE_TEMPLATE)];
};

CT_FIXTURE_SETUP(fixture) {
	int fd;

	memcpy(fixture->tmpfile, TMPFILE_TEMPLATE, sizeof(tmpfile));
	(void)mkstemp(fixture->tmpfile);

	fd = open(fixture->tmpfile, O_WRONLY);
	fixture->orig_stdout_fd = dup(STDOUT_FILENO);
	(void)close(STDOUT_FILENO);
	dup2(fd, STDOUT_FILENO);
	(void)close(fd);
}

CT_FIXTURE_TEARDOWN(fixture) {
        (void)unlink(fixture->tmpfile);
	if (fixture->orig_stdout_fd >= 0) {
		(void)close(STDOUT_FILENO);
		(void)dup2(fixture->orig_stdout_fd, STDOUT_FILENO);
	}
}

CT_FIXTURE(fixture);
```

*Note that this example has been significantly simplified by removing all error
handling. The complete fixture can be found in
[`src/examples/suite_with_fixtures.c`](../../src/examples/suite_with_fixtures.c).*

`CT_FIXTURE_TYPE` defines the state `struct` for the fixture. The block that
follows will become the definition of a `struct`, so it should come as no
surprise that the expression looks very much like that of a `struct`.  The
`CT_FIXTURE_TYPE` macro's only parameter is the name of the fixture; this must
match the name supplied to the other components of the fixture.

For this example, the fixture needs the name of the file to which `stdout` has
been redirected (the `tmpfile` field) and the file descriptor to where the
original `stdout` was saved (the `orig_stdout_fd` field). This will become more
obvious after looking at the set up and tear down routines.

`CT_FIXTURE_SETUP` and `CT_FIXTURE_TEARDOWN` define the set up and tear down
routines, respectively. The block that follows each of these macros will become
the definition of a function, so it should come as no surprise that the
expression that follows looks like a function body. The `CT_FIXTURE_SETUP` and
`CT_FIXTURE_TEARDOWN` macros' only parameter is the name of the fixture; this
must match the name supplied to the other components of the fixture.  A pointer
to the fixture to set up/tear down can be accessed via the `fixture` variable.

For this example, the fixture is initialized by creating a temporary file,
saving a reference to the existing `stdout` file descriptor, and redirecting
`stdout` to the temporary file. The fixture itself is updated by referencing
`fixture->tmpfile` and `fixture->orig_stdout_fd`.

`CT_FiXTURE` collects the set up and tear down routines in a form that can be
referenced from one or more tests. The macro's only parameter is the name of the
fixture; this must match the name supplied to the other components of the
fixture.

## Using a Fixture with a Test

A test that relies on a fixture looks very much like any other test except that
it is defined using the `CT_TEST_WITH_FIXTURE` macro. `CT_TEST_WITH_FIXTURE` is
similar to `CT_TEST` except that it takes an additional parameter indicating the
fixture what will supply state to the test. The fixture can be referenced from
within the body of the test through the `fixture` variable.

To test our `hello_world()` function, we might define the test as follows:

```c
CT_TEST_WITH_FIXTURE(hello_world, fixture)
{
	hello_world();
	verify_contents(fixture->tmpfile, "Hello, World!\n");
}
```

In this example, the name of the test is `hello_world` while the name of the
fixture supplying state is `fixture`. When the body of the test is executed, the
fixture will be initialized as described by the associated set up routine and
after the body of the function has completed the fixture will be finalized as
described by the associated tear down routine. A helper function,
`verify_contents()` has been defined to make sure the contents of the file exactly
matches the supplied `const char *` (more on this below).

Notice how the name of the fixture and the variable pointing to the fixture are
both `fixture`. This is a coincidence because the fixture was defined above with
`CT_FIXTURE` using the name `fixture`. The fixture is always referred to in the
body of the test using the variable `fixture`, regardless of what the fixture is
named.

The `C_TEST_WITH_FIXTURE` is similar to `CT_TEST` except it takes an additional
parameter. The first parameter is the name of the test while the second
parameter is the name of the fixture. When defined this way, the body of the
test may refer to the fixture using a variable named `fixture`.

Fixture-enabled tests are registered with using the `CT_SUITE_TEST` macro in the
same way as other tests:

```c
CT_SUITE_TESTS(hello_world) {
	CT_SUITE_TEST(hello_world),
};
CT_SUITE(hello_world);
```

## Combining Fixtures and Data Providers

A test that relies on a fixture **and** is powered by a data provider looks very
much like any other test except that it is defined using the
`CT_TEST_WITH_FIXTURE_AND_DATA` macro. `CT_TEST_WITH_FIXTURE_AND_DATA` is
similar to `CT_TEST_WITH_FIXTURE` except that it takes an additional parameter
indicating the data provider that will provide data for each test case.

To test our `hello_person()` function, we might define a data provider and test
as follows:

```c
CT_DATA_TYPE(hello_person) {
	const char *name;
	const char *expected;
};

CT_DATA(hello_person) {
	{ "Erich Gamma",     "Hello, Erich Gamma!\n" },
	{ "Richard Helm",    "Hello, Richard Helm!\n" },
	{ "Ralph Johnson",   "Hello, Ralph Johnson!\n" },
	{ "John Vlissides",  "Hello, John Vlissides!\n" },
};

CT_DATA_PROVIDER(hello_person, "%s", data->name);

CT_TEST_WITH_FIXTURE_AND_DATA(hello_person, fixture, hello_person) {
	hello_person(data->name);
	verify_contents(fixture->tmpfile, data->expected);
}
```

We'll skip over the details of defining the data provider since that was covered
[earlier](tutorial_data_providers.md).

Notice how we're reusing the fixture from the `hello_world` test? Fixtures can
easily be attached to any test. It's important to note that state of fixture is
not shared between tests; each test case gets it's own state. This means that
the `hello_world` test will not interfere with the `hello_person` test. Not only
that, but each test case of the `hello_person` test will not interfere with the
other test cases (e.g., the `"Erich Gamma"` case will be isolated from the
`"Richard Helm"` case).

The third parameter to the `CT_TEST_WITH_FIXTURE_AND_DATA` describes the name of
the data provider, `hello_person` in this case.

Fixture-and-data-provider-enabled tests are registered with using the
`CT_SUITE_TEST` macro in the same way as other tests:

```c
CT_SUITE_TESTS(hello_world) {
	CT_SUITE_TEST(hello_world),
	CT_SUITE_TEST(hello_person),
};
CT_SUITE(hello_world);
```

## Pulling it all together

if we combine this all together and define the missing `verify_contents()`
function, the complete test suite may look similar to the following:

```c
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <ctest/tests.h>

#include "hello_world.h"

/* Fixture */
#define TMPFILE_TEMPLATE "/tmp/fixtures-XXXXXXXX"

CT_FIXTURE_TYPE(fixture) {
	int orig_stdout_fd;
	char tmpfile[sizeof(TMPFILE_TEMPLATE)];
};

CT_FIXTURE_SETUP(fixture) {
	int new_stdout_fd;
	int orig_stdout_fd;
	char tmpfile[] = TMPFILE_TEMPLATE;

	fixture->orig_stdout_fd = -1;

	if (mkstemp(tmpfile) < 0) {
		CT_FAIL("Unable to create temporary file: %s", strerror(errno));
	}

	if ((new_stdout_fd = open(tmpfile, O_WRONLY)) < 0) {
		unlink(tmpfile);
		CT_FAIL("Unable to open temporary file: %s", strerror(errno));
	}

	if ((orig_stdout_fd = dup(STDOUT_FILENO)) < 0) {
		(void)close(new_stdout_fd);
		(void)unlink(tmpfile);
		CT_FAIL("Unable to dup STDOUT: %s", strerror(errno));
	}

	(void)close(STDOUT_FILENO);
	if (dup2(new_stdout_fd, STDOUT_FILENO) < 0) {
		(void)close(new_stdout_fd);
		(void)unlink(tmpfile);
		(void)dup2(orig_stdout_fd, STDOUT_FILENO);
		CT_FAIL("Unable to dup (fd=%d) to STDOUT: %s", new_stdout_fd, strerror(errno));
	}

	(void)close(new_stdout_fd);
	fixture->orig_stdout_fd = orig_stdout_fd;
	memcpy(fixture->tmpfile, tmpfile, sizeof(tmpfile));
}

CT_FIXTURE_TEARDOWN(fixture) {
	if (fixture->tmpfile[0] != '\0')
		(void)unlink(fixture->tmpfile);
	if (fixture->orig_stdout_fd >= 0) {
		(void)close(STDOUT_FILENO);
		(void)dup2(fixture->orig_stdout_fd, STDOUT_FILENO);
	}
}

CT_FIXTURE(fixture);

static void verify_contents(const char *filename, const char *expected) {
	const int expected_len = strlen(expected);
	int fd, len;
	char data[expected_len + 1];

	/* stdout might be buffered */
	fflush(stdout);

	if ((fd = open(filename, O_RDONLY)) < 0)
		CT_FAIL("Unable to open output file: %s", strerror(errno));

	if ((len = read(fd, data, sizeof(data)-1)) < 0)
		CT_FAIL("Unable to read from output file: %s", strerror(errno));
	data[len] = '\0';

	/* Data should be the same and no more data should be available. */
	CT_ASSERT_STR_EQ(data, expected);
	CT_ASSERT_INT_EQ(read(fd, data, 1), 0);
}

CT_TEST_WITH_FIXTURE(hello_world, fixture)
{
	hello_world();
	verify_contents(fixture->tmpfile, "Hello, World!\n");
}


CT_DATA_TYPE(hello_person) {
	const char *name;
	const char *expected;
};

CT_DATA(hello_person) {
	{ "Erich Gamma",     "Hello, Erich Gamma!\n" },
	{ "Richard Helm",    "Hello, Richard Helm!\n" },
	{ "Ralph Johnson",   "Hello, Ralph Johnson!\n" },
	{ "John Vlissides",  "Hello, John Vlissides!\n" },
};

CT_DATA_PROVIDER(hello_person, "%s", data->name);

CT_TEST_WITH_FIXTURE_AND_DATA(hello_person, fixture, hello_person) {
	hello_person(data->name);
	verify_contents(fixture->tmpfile, data->expected);
}


CT_SUITE_TESTS(hello_world) {
	CT_SUITE_TEST(hello_world),
	CT_SUITE_TEST(hello_person),
};
CT_SUITE(hello_world);
```

(This test file can be found in
[`src/examples/suite_with_fixtures.c`](../../src/examples/suite_with_fixtures.c)).

## Running Tests with Fixtures

Running tests that use fixtures is no different than non-fixture test cases.

You can run the compiled examples as follows:

```
$ src/cli/ctester run src/examples/suite_with_fixtures.la
hello_world:hello_world ... OK
hello_world:hello_person[Erich Gamma] ... OK
hello_world:hello_person[Richard Helm] ... OK
hello_world:hello_person[Ralph Johnson] ... OK
hello_world:hello_person[John Vlissides] ... OK
```
