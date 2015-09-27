#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctest/_annotations.h>
#include <ctest/exec/suite.h>
#include "utils.h"

CTEST_VPRINTF__(1)
static char *vsnprintfdup(const char *fmt, va_list args) {
	va_list args_copy;
	char *str = NULL;
	int len;
	/* args can't be used twice, must make a copy */
	va_copy(args_copy, args);
       	len = vsnprintf(NULL, 0, fmt, args_copy);
	va_end(args_copy);

	if (len >= 0) {
		str = malloc(len+1);
		if (str != NULL)
			vsnprintf(str, len+1, fmt, args);
	}
	return str;
}

/*
 * Test Case
 */
typedef struct testcase__ testcase_t__;
struct testcase__ {
	ctest_testcase_t base;
	const char *name;
	ctest_test_t *test;
};

static inline testcase_t__ *upcast_testcase__(ctest_testcase_t *testcase) {
	return containerof(testcase, testcase_t__, base);
}

static const char *testcase_op_get_name__(ctest_testcase_t *ctest_testcase) {
	testcase_t__ *const testcase = upcast_testcase__(ctest_testcase);
	return testcase->name;
}

static ctest_test_t *testcase_op_get_test__(ctest_testcase_t *ctest_testcase) {
	testcase_t__ *const testcase = upcast_testcase__(ctest_testcase);
	return testcase->test;
}

CTEST_PRINTF__(5, 6)
static void exec_hooks_on_skip__(ctest_exec_hooks_t *hooks, ctest_stage_t stage, const char *file, int line, const char *fmt, ...)
{
	const ctest_location_t location = {file, line };
	va_list fmt_params;
	ctest_failure_t *failure;

	va_start(fmt_params, fmt);
	failure = ctest_failure_create_va(stage, fmt, fmt_params, &location, NULL);
	va_end(fmt_params);

	ctest_exec_hooks_on_skip(hooks, failure);
}

CTEST_PRINTF__(5, 6)
static void exec_hooks_on_failure__(ctest_exec_hooks_t *hooks, ctest_stage_t stage, const char *file, int line, const char *fmt, ...)
{
	const ctest_location_t location = {file, line };
	va_list fmt_params;
	ctest_failure_t *failure;

	va_start(fmt_params, fmt);
	failure = ctest_failure_create_va(stage, fmt, fmt_params, &location, NULL);
	va_end(fmt_params);

	ctest_exec_hooks_on_failure(hooks, failure);
}

static void testcase_op_execute_success__(ctest_testcase_t *unused(testcase), ctest_exec_hooks_t *unused(hooks)) {
}

static void testcase_op_execute_skip__(ctest_testcase_t *unused(testcase), ctest_exec_hooks_t *hooks) {
	exec_hooks_on_skip__(hooks, CTEST_STAGE_SETUP, __FILE__, __LINE__, "skip test");
}

static void testcase_op_execute_setup_failure__(ctest_testcase_t *ctest_testcase, ctest_exec_hooks_t *hooks) {
	testcase_t__ *const testcase = upcast_testcase__(ctest_testcase);
	fprintf(stdout, "STDOUT: setup failure line 1\n");
	fprintf(stdout, "STDOUT: setup failure line 2\n");
	fprintf(stdout, "STDOUT: setup failure line 3\n");
	fprintf(stderr, "STDERR: setup failure line 1\n");
	fprintf(stderr, "STDERR: setup failure line 2\n");
	fprintf(stderr, "STDERR: setup failure line 3\n");
	exec_hooks_on_failure__(hooks, CTEST_STAGE_SETUP, __FILE__, __LINE__, "failed to set up \"%s\"", testcase->name);
}

static void testcase_op_execute_teardown_failure__(ctest_testcase_t *ctest_testcase, ctest_exec_hooks_t *hooks) {
	testcase_t__ *const testcase = upcast_testcase__(ctest_testcase);
	fprintf(stdout, "STDOUT: teardown failure line 1\n");
	fprintf(stdout, "STDOUT: teardown failure line 2\n");
	fprintf(stdout, "STDOUT: teardown failure line 3\n");
	fprintf(stderr, "STDERR: teardown failure line 1\n");
	fprintf(stderr, "STDERR: teardown failure line 2\n");
	fprintf(stderr, "STDERR: teardown failure line 3\n");
	exec_hooks_on_failure__(hooks, CTEST_STAGE_TEARDOWN, __FILE__, __LINE__, "failed to tear down \"%s\"", testcase->name);
}

static void testcase_op_execute_failure__(ctest_testcase_t *ctest_testcase, ctest_exec_hooks_t *hooks) {
	testcase_t__ *const testcase = upcast_testcase__(ctest_testcase);
	fprintf(stdout, "STDOUT: failure line 1\n");
	fprintf(stdout, "STDOUT: failure line 2\n");
	fprintf(stdout, "STDOUT: failure line 3\n");
	fprintf(stderr, "STDERR: failure line 1\n");
	fprintf(stderr, "STDERR: failure line 2\n");
	fprintf(stderr, "STDERR: failure line 3\n");
	exec_hooks_on_failure__(hooks, CTEST_STAGE_EXECUTION, __FILE__, __LINE__, "failed to execute \"%s\"", testcase->name);
}

static void testcase_destroy__(testcase_t__ *testcase) {
	(void)free((char *)testcase->name);   /* const-cast */
	memset(testcase, 0, sizeof(*testcase));
	(void)free(testcase);
}

static ctest_testcase_ops_t testcase_success_ops__ = {
	&testcase_op_get_name__,
	&testcase_op_get_test__,
	&testcase_op_execute_success__,
};

static ctest_testcase_ops_t testcase_skip_ops__ = {
	&testcase_op_get_name__,
	&testcase_op_get_test__,
	&testcase_op_execute_skip__,
};

static ctest_testcase_ops_t testcase_setup_failure_ops__ = {
	&testcase_op_get_name__,
	&testcase_op_get_test__,
	&testcase_op_execute_setup_failure__,
};

static ctest_testcase_ops_t testcase_teardown_failure_ops__ = {
	&testcase_op_get_name__,
	&testcase_op_get_test__,
	&testcase_op_execute_teardown_failure__,
};

static ctest_testcase_ops_t testcase_failure_ops__ = {
	&testcase_op_get_name__,
	&testcase_op_get_test__,
	&testcase_op_execute_failure__,
};

CTEST_PRINTF__(3, 4) CTEST_NONNULL_ARGS__(1, 2, 3)
static testcase_t__ *testcase_create__(ctest_testcase_ops_t *ops, ctest_test_t *test, const char *fmt, ...) {
	va_list args;
	testcase_t__ *testcase;
	char *name;

	va_start(args, fmt);
	name = vsnprintfdup(fmt, args);
	va_end(args);
	if (name == NULL)
		goto create_name_failed;

	if ((testcase = calloc(1, sizeof(*testcase))) == NULL)
		goto alloc_tc_failed;

	testcase->base.ops = ops;
	testcase->name = name;
	testcase->test = test;
	return testcase;

alloc_tc_failed:
	(void)free(name);
create_name_failed:
	return NULL;
}

/*
 * Test
 */
typedef struct test__ test_t__;
struct test__ {
	ctest_test_t base;
	const char *name;
	ctest_testsuite_t *testsuite;
	ctest_testcase_t **testcases;
	size_t testcase_count;
};

static inline test_t__ *upcast_test__(ctest_test_t *test) {
	return containerof(test, test_t__, base);
}

static const char *test_op_get_name__(ctest_test_t *ctest_test) {
	test_t__ *const test = upcast_test__(ctest_test);
	return test->name;
}

static ctest_testsuite_t *test_op_get_testsuite__(ctest_test_t *ctest_test) {
	test_t__ *const test = upcast_test__(ctest_test);
	return test->testsuite;
}

static size_t test_op_get_testcase_count__(ctest_test_t *ctest_test) {
	test_t__ *const test = upcast_test__(ctest_test);
	return test->testcase_count;
}

static ctest_testcase_t *const*test_op_get_testcases__(ctest_test_t *ctest_test) {
	test_t__ *const test = upcast_test__(ctest_test);
	return test->testcases;
}

static void test_destroy__(test_t__ *test) {
	size_t i;

	for (i = 0; i < test->testcase_count; ++i) {
		testcase_destroy__(upcast_testcase__(test->testcases[i]));
	}
	(void)free(test->testcases);
	(void)free((char *)test->name); /* const-cast */
	memset(test, 0, sizeof(*test));
	(void)free(test);
}

CTEST_PRINTF__(3, 4) CTEST_NONNULL_ARGS__(2, 3)
static test_t__ *test_create__(size_t testcase_count, ctest_testsuite_t *testsuite, const char *fmt, ...) {
	static ctest_test_ops_t ops = {
		&test_op_get_name__,
		&test_op_get_testsuite__,
		&test_op_get_testcase_count__,
		&test_op_get_testcases__,
	};

	va_list args;
	test_t__ *test;
	ctest_testcase_t **testcases;
	char *name;

	va_start(args, fmt);
	name = vsnprintfdup(fmt, args);
	va_end(args);
	if (name == NULL)
		goto create_name_failed;

	if ((testcases = calloc(testcase_count, sizeof(*testcases))) == NULL)
		goto alloc_testcases_failed;

	if ((test = calloc(1, sizeof(*test))) == NULL)
		goto alloc_t_failed;

	test->base.ops = &ops;
	test->name = name;
	test->testsuite = testsuite;
	test->testcases = testcases;
	test->testcase_count = 0;
	return test;

alloc_t_failed:
	(void)free(testcases);
alloc_testcases_failed:
	(void)free(name);
create_name_failed:
	return NULL;
}

/*
 * Test Suite
 */
typedef struct testsuite__ testsuite_t__;
struct testsuite__ {
	ctest_testsuite_t base;
	const char *name;
	ctest_test_t **tests;
	size_t test_count;
};

static inline testsuite_t__ *upcast_testsuite__(ctest_testsuite_t *testsuite) {
	return containerof(testsuite, testsuite_t__, base);
}

static const char *testsuite_op_get_name__(ctest_testsuite_t *ctest_testsuite) {
	testsuite_t__ * const testsuite = upcast_testsuite__(ctest_testsuite);
	return testsuite->name;
}

static size_t testsuite_op_get_test_count__(ctest_testsuite_t *ctest_testsuite) {
	testsuite_t__ * const testsuite = upcast_testsuite__(ctest_testsuite);
	return testsuite->test_count;
}

static ctest_test_t *const*testsuite_op_get_tests__(ctest_testsuite_t *ctest_testsuite) {
	testsuite_t__ * const testsuite = upcast_testsuite__(ctest_testsuite);
	return testsuite->tests;
}

static void testsuite_op_destroy__(ctest_testsuite_t *ctest_testsuite) {
	testsuite_t__ * const testsuite = upcast_testsuite__(ctest_testsuite);
	size_t i;

	for (i = 0; i < testsuite->test_count; ++i) {
		test_destroy__(upcast_test__(testsuite->tests[i]));
	}
	(void)free(testsuite->tests);
	(void)free((char *)testsuite->name);   /* const-cast */
	memset(testsuite, 0, sizeof(*testsuite));
	(void)free(testsuite);
}

CTEST_PRINTF__(2, 3) CTEST_NONNULL_ARGS__(2)
static testsuite_t__ *testsuite_create__(size_t test_count, const char *fmt, ...) {
	static ctest_testsuite_ops_t ops = {
		&testsuite_op_get_name__,
		&testsuite_op_get_test_count__,
		&testsuite_op_get_tests__,
		&testsuite_op_destroy__,
	};

	va_list args;
	testsuite_t__ *testsuite;
	ctest_test_t **tests;
	char *name;

	va_start(args, fmt);
	name = vsnprintfdup(fmt, args);
	va_end(args);
	if (name == NULL)
		goto create_name_failed;

	if ((tests = calloc(test_count, sizeof(*tests))) == NULL)
		goto alloc_tests_failed;

	if ((testsuite = calloc(1, sizeof(*testsuite))) == NULL)
		goto alloc_t_failed;

	testsuite->base.ops = &ops;
	testsuite->name = name;
	testsuite->tests = tests;
	testsuite->test_count = 0;
	return testsuite;

alloc_t_failed:
	(void)free(tests);
alloc_tests_failed:
	(void)free(name);
create_name_failed:
	return NULL;
}

/**
 * Create a test suite for testing other components.
 *
 * This test suite defines test suites with different properties in order to
 * run through implementations of runners, reporters, etc.
 *
 * \return A test suite suitable for testing frameworks.
 */
ctest_testsuite_t *ctest_create_testing_testsuite(const char *name) {
	static const struct { const char *name; ctest_testcase_ops_t *ops; } testcases[] = {
		{ "success", &testcase_success_ops__, },
		{ "skip", &testcase_skip_ops__, },
		{ "setup_failure", &testcase_setup_failure_ops__, },
		{ "teardown_failure", &testcase_teardown_failure_ops__, },
		{ "failure", &testcase_failure_ops__, },
	};
	static const size_t testcase_count = sizeof(testcases) / sizeof(*testcases);
	const size_t test_count = (1 << testcase_count) - 1;
	size_t i_test;
	testsuite_t__ *testsuite;

	if ((testsuite = testsuite_create__(test_count, "testsuite_%s", name)) == NULL)
		goto create_suite_failure;

	for (i_test = 0; i_test < test_count; ++i_test) {
		size_t i_testcase;
		test_t__ *test;
		if ((test = test_create__(testcase_count, &testsuite->base, "test_%s_%zd", name, i_test+1)) == NULL)
			goto create_test_failed;
		testsuite->tests[testsuite->test_count++] = &test->base;

		for (i_testcase = 0; i_testcase < testcase_count; ++i_testcase) {
			testcase_t__ *testcase;
			if (!((i_test + 1) & (1 << i_testcase)))
				continue;
			if ((testcase = testcase_create__(testcases[i_testcase].ops, &test->base, "testcase_%s_%zd_%s", name, i_test+1, testcases[i_testcase].name)) == NULL)
				goto create_test_failed;
			test->testcases[test->testcase_count++] = &testcase->base;
		}
	}

	return &testsuite->base;
create_test_failed:
	ctest_testsuite_destroy(&testsuite->base);
create_suite_failure:
	return NULL;
}
