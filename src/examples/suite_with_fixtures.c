#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
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
