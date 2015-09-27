#include <errno.h>
#include <execinfo.h>
#include <ltdl.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <ctest/_annotations.h>
#include <ctest/_preprocessor.h>
#include <ctest/exec/exec_hooks.h>
#include <ctest/exec/failure.h>
#include <ctest/exec/location.h>
#include <ctest/exec/suite.h>
#include <ctest/tests/tests.h>

#include "dynamic_ops.h"
#include "utils.h"

/*
 * Test Suite Structures
 */
typedef struct testcase__ testcase_t__;
typedef struct test__ test_t__;
typedef struct testsuite__ testsuite_t__;

struct testcase__ {
	ctest_testcase_t base;
	test_t__ *test;
	const char *name;
	const void *data;
};

struct test__ {
	ctest_test_t base;
	testsuite_t__ *testsuite;
	ctest_def_test_t__ *def;
	ctest_testcase_t *const*testcases;
	size_t testcase_count;
};

struct testsuite__ {
	ctest_testsuite_t base;
	lt_dlhandle handle;
	ctest_def_suite_t__ *def;
	ctest_dynamic_ops_t **p_dynamic_ops;
	ctest_test_t *const*tests;
	size_t test_count;
};

/*
 * Dynamic Operations
 */
typedef struct loader_dynamic_ops__ loader_dynamic_ops_t__;
struct loader_dynamic_ops__ {
	ctest_dynamic_ops_t base;
	ctest_exec_hooks_t *hooks;
	ctest_dynamic_ops_t **p_dynamic_ops;
	ctest_dynamic_ops_t *old_dynamic_ops;
	ctest_failure_t *failure;
	ctest_stage_t stage;
	void *fixture;
	void (*teardown)(void *);
	ctest_dynamic_ops_abort_type_t abort_type;
	bool free_fixture;
};

static inline loader_dynamic_ops_t__ *upcast_dynamic_ops__(ctest_dynamic_ops_t *dynamic_ops)
{
	return containerof(dynamic_ops, loader_dynamic_ops_t__, base);
}

CTEST_NORETURN__
static void dynamic_ops_abort__(loader_dynamic_ops_t__ *dynamic_ops, ctest_dynamic_ops_abort_type_t abort_type)
{
	ctest_failure_t *failure;

	if (dynamic_ops->abort_type == CTEST_DYNAMIC_OPS_ABORT_NONE) {
		/* This abort must be happening within another abort (i.e., the
		 * teardown function is aborting while it is being called to
		 * clean up during an abort).
		 *
		 * Maintain the existing abort type so that a failure in the
		 * teardown can't promote a skip to a failure and a skip in the
		 * teardown can't demote a failure to a skip. */
		dynamic_ops->abort_type = abort_type;
	}

	if (dynamic_ops->teardown != NULL) {
		/* If the teardown function raises attempts to abort, this
		 * function will get called again. To prevent infinite
		 * recursion, clear the teardown function first. */
		void (*teardown)(void *) = dynamic_ops->teardown;
		dynamic_ops->teardown = NULL;
		(*teardown)(dynamic_ops->fixture);
	}

	if (dynamic_ops->free_fixture) {
		(void)free(dynamic_ops->fixture);
		dynamic_ops->fixture = NULL;
		dynamic_ops->free_fixture = false;
	}
	*dynamic_ops->p_dynamic_ops = dynamic_ops->old_dynamic_ops;     /* Restore original hook with the module. */

	failure = dynamic_ops->failure;
	dynamic_ops->failure = NULL;

	switch (dynamic_ops->abort_type) {
	case CTEST_DYNAMIC_OPS_ABORT_NONE:
	case CTEST_DYNAMIC_OPS_ABORT_FAIL:
		break;
	case CTEST_DYNAMIC_OPS_ABORT_SKIP:
		ctest_exec_hooks_on_skip(dynamic_ops->hooks, failure);
	}
	ctest_exec_hooks_on_failure(dynamic_ops->hooks, failure);
}

CTEST_VPRINTF__(4)
static void dynamic_ops_op_report_failure__(ctest_dynamic_ops_t *ctest_dynamic_ops, const char *file, int line, const char *fmt, va_list fmt_params)
{
	loader_dynamic_ops_t__ *const dynamic_ops = upcast_dynamic_ops__(ctest_dynamic_ops);

	/* If multiple errors are reported, keep the first one.
	 * TODO: Report all errors.
	 */
	if (dynamic_ops->failure == NULL) {
		ctest_location_t location = { file, line };
		dynamic_ops->failure = ctest_failure_create_va(dynamic_ops->stage, fmt, fmt_params, &location, NULL);
	}

}

CTEST_NORETURN__
static void dynamic_ops_op_abort__(ctest_dynamic_ops_t *ctest_dynamic_ops, ctest_dynamic_ops_abort_type_t abort_type)
{
	dynamic_ops_abort__(upcast_dynamic_ops__(ctest_dynamic_ops), abort_type);
}

/*
 * Null Data Provider
 */

static int null_data_provider_to_string__(char *buf, size_t len, const void *unused(data))
{
	if (len > 0)
		buf[0] = '\0';
	return 0;
}

static ctest_def_data_provider_t__ null_data_provider__ = {
	NULL,
	1,
	0,
	&null_data_provider_to_string__,
};

/*
 * Test case
 */

static inline testcase_t__ *upcast_testcase__(ctest_testcase_t *testcase)
{
	return containerof(testcase, testcase_t__, base);
}

CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static const char *testcase_op_get_name__(ctest_testcase_t *ctest_testcase)
{
	testcase_t__ *const testcase = upcast_testcase__(ctest_testcase);
	return testcase->name;
}

CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static ctest_test_t *testcase_op_get_test__(ctest_testcase_t *ctest_testcase)
{
	testcase_t__ *const testcase = upcast_testcase__(ctest_testcase);
	return &testcase->test->base;
}

CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static void testcase_op_execute__(ctest_testcase_t *ctest_testcase, ctest_exec_hooks_t *hooks)
{
	static ctest_dynamic_ops_ops_t ops = {
		&dynamic_ops_op_report_failure__,
		&dynamic_ops_op_abort__,
	};
	static ctest_def_fixture_provider_t__ default_fixture_provider = { NULL, NULL, 0, };

	testcase_t__ *const testcase = upcast_testcase__(ctest_testcase);
	test_t__*const test = testcase->test;
	testsuite_t__*const testsuite = test->testsuite;
	ctest_def_test_t__ *const test_def = test->def;
	ctest_def_fixture_provider_t__ *const fixture_provider = test_def->fixture_provider ?: &default_fixture_provider;
	loader_dynamic_ops_t__ dynamic_ops;
	char fixture_storage[128];

	dynamic_ops.base.ops = &ops;
	dynamic_ops.hooks = hooks;
	dynamic_ops.p_dynamic_ops = testsuite->p_dynamic_ops;
	dynamic_ops.failure = NULL;
	dynamic_ops.fixture = NULL;
	dynamic_ops.teardown = fixture_provider->teardown;
	dynamic_ops.abort_type = CTEST_DYNAMIC_OPS_ABORT_NONE;
	dynamic_ops.free_fixture = false;

	/* Hook us into how the module reports failures (it's automatically
	 * unhooked on failure). */
	if (testsuite->p_dynamic_ops != NULL) {
		dynamic_ops.old_dynamic_ops = *(testsuite->p_dynamic_ops);
		*(testsuite->p_dynamic_ops) = &dynamic_ops.base;
	}

	dynamic_ops.stage = CTEST_STAGE_SETUP;
	ctest_exec_hooks_on_stage_change(hooks, CTEST_STAGE_SETUP);

	if (fixture_provider->size > sizeof(fixture_storage)) {
		if ((dynamic_ops.fixture = calloc(1, fixture_provider->size)) == NULL) {
			ctest_exec_hooks_on_failure(hooks, ctest_failure_create(CTEST_STAGE_SETUP, "fixture allocation failure: %s", NULL, NULL, strerror(errno)));
			return;
		}
		dynamic_ops.free_fixture = true;
	} else {
		dynamic_ops.fixture = fixture_storage;
		memset(fixture_storage, 0, sizeof(fixture_storage));
	}

	if (fixture_provider->setup != NULL)
		(*fixture_provider->setup)(dynamic_ops.fixture);

	dynamic_ops.stage = CTEST_STAGE_EXECUTION;
	ctest_exec_hooks_on_stage_change(hooks, CTEST_STAGE_EXECUTION);
	(*test_def->caller)(dynamic_ops.fixture, testcase->data);

	dynamic_ops.stage = CTEST_STAGE_TEARDOWN;
	ctest_exec_hooks_on_stage_change(hooks, CTEST_STAGE_TEARDOWN);
	if (dynamic_ops.teardown != NULL) {
		/* By clearing out the teardown function from dynamic_ops, if
		 * the teardown function results in a error, we won't attempt
		 * to call it again when handling the error. */
		void (*teardown)(void *) = dynamic_ops.teardown;
		dynamic_ops.teardown = NULL;
		(*teardown)(dynamic_ops.fixture);
	}
	if (dynamic_ops.failure != NULL) {
		/* An error was reported during the teardown but not in
		 * conjunction with a abort; promote to an abort. */
		dynamic_ops_abort__(&dynamic_ops, CTEST_DYNAMIC_OPS_ABORT_FAIL);
	}

	/* Remove dynamic operations from module for reporting failures. */
	if (testsuite->p_dynamic_ops != NULL) {
		*(testsuite->p_dynamic_ops) = dynamic_ops.old_dynamic_ops;
	}

	if (dynamic_ops.free_fixture)
		(void)free(dynamic_ops.fixture);
}

static testcase_t__ *testcase_create__(test_t__*test, ctest_def_data_provider_t__ *data_provider, size_t index)
{
	static ctest_testcase_ops_t ops__ = {
		&testcase_op_get_name__,
		&testcase_op_get_test__,
		&testcase_op_execute__,
	};
	ctest_def_test_t__ *const test_def = test->def;
	const void *const data = data_provider->data + (index * data_provider->size);
	testcase_t__ *result;
	int data_length;
	const char *name;

	if ((result = calloc(1, sizeof(*result))) == NULL)
		goto alloc_failed;

	data_length = (*data_provider->to_string)(NULL, 0, data);
	if (data_length > 0) {
		const int name_length = strlen(test_def->name);
		/* format name[data] */
		int length = name_length + 1 + data_length + 1 + 1;
		char *buf;

		if ((buf = malloc(length)) == NULL)
			goto name_alloc_failed;
		name = buf;

		memcpy(buf, test_def->name, name_length);
		buf += name_length;
		length -= name_length;

		*(buf++) = '[';
		length--;

		(*data_provider->to_string)(buf, length, data);
		buf += data_length;
		length -= data_length;

		*(buf++) = ']';
		*(buf++) = '\0';
	} else {
		/* The destructor won't free it if it's a copy */
		name = test_def->name;
	}

	result->base.ops = &ops__;
	result->test = test;
	result->name = name;
	result->data = data;
	return result;

name_alloc_failed:
	(void)free(result);
alloc_failed:
	return NULL;
}

static void testcase_destroy__(testcase_t__ *testcase)
{
	if (testcase->name != testcase->test->def->name)
		(void)free((char *)testcase->name);
	memset(testcase, 0, sizeof(*testcase));
	(void)free(testcase);
}

/*
 * Tests
 */

static inline test_t__ *upcast_test__(ctest_test_t *test) {
	return containerof(test, test_t__, base);
}

CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static const char *test_op_get_name__(ctest_test_t *ctest_test)
{
	test_t__ *const test = upcast_test__(ctest_test);
	return test->def->name;
}

CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static ctest_testsuite_t *test_op_get_testsuite__(ctest_test_t *ctest_test)
{
	test_t__ *const test = upcast_test__(ctest_test);
	return &test->testsuite->base;
}

CTEST_ALL_NONNULL_ARGS__
static size_t test_op_get_testcase_count__(ctest_test_t *ctest_test)
{
	test_t__ *const test = upcast_test__(ctest_test);
	return test->testcase_count;
}

CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static ctest_testcase_t *const*test_op_get_testcases__(ctest_test_t *ctest_test)
{
	test_t__ *const test = upcast_test__(ctest_test);
	return test->testcases;
}

static test_t__ *test_create__(testsuite_t__ *testsuite, ctest_def_test_t__ *test_def)
{
	static ctest_test_ops_t ops = {
		&test_op_get_name__,
		&test_op_get_testsuite__,
		&test_op_get_testcase_count__,
		&test_op_get_testcases__,
	};

	ctest_def_data_provider_t__ *const data_provider = test_def->data_provider ? test_def->data_provider : &null_data_provider__;
	test_t__ *result;
	ctest_testcase_t **testcases;
	size_t i;

	if ((result = calloc(1, sizeof(*result))) == NULL)
		goto alloc_failed;

	if ((testcases = calloc(data_provider->count, sizeof(*testcases))) == NULL)
		goto testcases_alloc_failed;

	result->base.ops = &ops;
	result->def = test_def;
	result->testsuite = testsuite;
	result->testcases = testcases;
	result->testcase_count = data_provider->count;

	for (i = 0; i < data_provider->count; ++i) {
		testcase_t__ *testcase;
		if ((testcase = testcase_create__(result, data_provider, i)) == NULL)
			goto testcase_create_failed;
		testcases[i] = &testcase->base;
	}

	return result;

testcase_create_failed:
	for (i = 0; i < data_provider->count; ++i) {
		if (testcases[i] != NULL)
			testcase_destroy__(upcast_testcase__(testcases[i]));
	}
	(void)free(testcases);
testcases_alloc_failed:
	(void)free(result);
alloc_failed:
	return NULL;
}

static void test_destroy__(test_t__ *test)
{
	ctest_testcase_t **const testcases = (ctest_testcase_t **)test->testcases;
	size_t i;
	for (i = 0; i < test->testcase_count; ++i) {
		testcase_destroy__(upcast_testcase__(testcases[i]));
	}
	(void)free(testcases);
	memset(test, 0, sizeof(*test));
	(void)free(test);
}

/*
 * Test Suite
 */

static inline testsuite_t__ *upcast_testsuite__(ctest_testsuite_t *testsuite) {
	return containerof(testsuite, testsuite_t__, base);
}

CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static const char *testsuite_op_get_name__(ctest_testsuite_t *ctest_testsuite)
{
	testsuite_t__ *const testsuite = upcast_testsuite__(ctest_testsuite);
	return testsuite->def->name;
}

CTEST_ALL_NONNULL_ARGS__
static size_t testsuite_op_get_test_count__(ctest_testsuite_t *ctest_testsuite)
{
	testsuite_t__ *const testsuite = upcast_testsuite__(ctest_testsuite);
	return testsuite->test_count;
}

CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static ctest_test_t *const*testsuite_op_get_tests__(ctest_testsuite_t *ctest_testsuite)
{
	testsuite_t__ *const testsuite = upcast_testsuite__(ctest_testsuite);
	return testsuite->tests;
}

CTEST_ALL_NONNULL_ARGS__
static void testsuite_op_destroy__(ctest_testsuite_t *ctest_testsuite)
{
	testsuite_t__ *const testsuite = upcast_testsuite__(ctest_testsuite);
	ctest_test_t **tests = (ctest_test_t **)testsuite->tests;
	lt_dlhandle handle = testsuite->handle;
	size_t i;

	for (i = 0; i < testsuite->test_count; ++i) {
		test_destroy__(upcast_test__(tests[i]));
	}

	(void)free(tests);
	memset(testsuite, 0, sizeof(*testsuite));
	(void)free(testsuite);

	lt_dlclose(handle);
	lt_dlexit();
}

ctest_testsuite_t *ctest_load_testsuite(const char *filename)
{
	static ctest_testsuite_ops_t ops = {
		&testsuite_op_get_name__,
		&testsuite_op_get_test_count__,
		&testsuite_op_get_tests__,
		&testsuite_op_destroy__,
	};

	lt_dladvise dladvise;
	lt_dlhandle dlhandle;
	void *sym;
	ctest_def_suite_t__ *suite_def;
	ctest_dynamic_ops_t **p_dynamic_ops;
	testsuite_t__ *result;
	ctest_test_t **tests;
	size_t i;

	if (lt_dlinit() != 0) {
		fprintf(stderr, "lt_dlinit() failed: %s\n", lt_dlerror());
		goto dlinit_failed;
	}

	/* Prefer local visibility only (symbols in module are not made
	 * available to satisfy unresolved symbols of other modules. */
	if (lt_dladvise_init(&dladvise) != 0) {
		fprintf(stderr, "lt_dladvise() failed: %s\n", lt_dlerror());
		goto dladvise_init_failed;
	}
	if (lt_dladvise_local(&dladvise) != 0) {
		fprintf(stderr, "lt_dladvise_local() failed: %s\n", lt_dlerror());
		goto dladvise_local_failed;
	}

	if ((dlhandle = lt_dlopenadvise(filename, dladvise)) == NULL) {
		fprintf(stderr, "lt_dlopen() failed: %s\n", lt_dlerror());
		goto dlopen_failed;
	}

	if ((sym = lt_dlsym(dlhandle, CTEST_STRINGIZE__(CTEST_SUITE_SYMBOL__))) == NULL) {
		fprintf(stderr, "module missing " CTEST_STRINGIZE__(CTEST_SUITE_SYMBOL__) "\n");
		goto dlsym_suite_failed;
	}

	suite_def = sym;
	if (suite_def->magic != CTEST_SUITE_MAGIC__) {
		fprintf(stderr, "module contains bad magic (found:0x%08x expecting:0x%08x)", suite_def->magic, CTEST_SUITE_MAGIC__);
		goto bad_sym;
	}
	if (suite_def->version != CTEST_SUITE_VERSION__) {
		fprintf(stderr, "module contains unknown magic (found:0x%08x expecting:0x%08x)", suite_def->version, CTEST_SUITE_VERSION__);
		goto bad_sym;
	}

	/* dynamic ops are optional and won't be present if the test module doesn't
	 * have any tests that can fail. */
	p_dynamic_ops = lt_dlsym(dlhandle, CTEST_STRINGIZE__(CTEST_DYNAMIC_OPS_SYMBOL__));

	if ((result = calloc(1, sizeof(*result))) == NULL)
		goto alloc_failed;

	if (suite_def->test_count > 0) {
		if ((tests = calloc(suite_def->test_count, sizeof(*tests))) == NULL)
			goto tests_alloc_failed;
	} else {
		tests = NULL;
	}

	result->base.ops = &ops;
	result->handle = dlhandle;
	result->def = suite_def;
	result->p_dynamic_ops = p_dynamic_ops;
	result->tests = tests;
	result->test_count = suite_def->test_count;

	for (i = 0; i < suite_def->test_count; ++i) {
		test_t__ *test;

		if ((test = test_create__(result, suite_def->tests[i])) == NULL)
			goto test_creation_failed;
		tests[i] = &test->base;
	}

	return &result->base;

test_creation_failed:
	for (i = 0; i < suite_def->test_count; ++i) {
		if (tests[i] != NULL)
			test_destroy__(upcast_test__(tests[i]));
	}
tests_alloc_failed:
	(void)free(result);
alloc_failed:
bad_sym:
dlsym_suite_failed:
	lt_dlclose(dlhandle);
dlopen_failed:
dladvise_local_failed:
	lt_dladvise_destroy(&dladvise);
dladvise_init_failed:
	lt_dlexit();
dlinit_failed:
	return NULL;
}
