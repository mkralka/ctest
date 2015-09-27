#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ctest/_annotations.h>
#include <ctest/exec.h>
#include "utils.h"

static void wrap_output__(FILE *fp, const char *prefix, const char *str)
{
	while (true) {
		char *ptr;
		if ((ptr = strchr(str, '\n')) == NULL)
			break;
		fprintf(fp, "%s%.*s\n", prefix, (int)(ptr-str), str);
		str = ptr + 1;
	}

	if (*str != '\0')
		fprintf(fp, "%s%s\n", prefix, str);
}

static FILE *dup__(FILE *fp)
{
	int fd;
	int dupfd;
	FILE *result;
	int mode;
	const char *str_mode;

	if ((fd = fileno(fp)) < 0)
		goto probe_failed;
	if ((mode = fcntl(fd, F_GETFL)) < 0)
		goto probe_failed;

	switch (mode & O_ACCMODE) {
	case O_RDONLY:
		str_mode = "r";
		break;
	case O_RDWR:
		str_mode = "r+";
		break;
	case O_WRONLY:
		str_mode = "w";
		break;
	default:
		goto probe_failed;
	}

	if ((dupfd = dup(fd)) < 0)
		goto dup_failed;

	/* Ensure this descriptor can't be accessed by child processes, that
	 * could be created by the runner. */
	if (fcntl(dupfd, F_SETFD, FD_CLOEXEC) == -1)
		goto dup_cloexec_failed;

	if ((result = fdopen(dupfd, str_mode)) == NULL)
		goto open_failed;

	return result;

open_failed:
dup_cloexec_failed:
	(void)close(dupfd);
dup_failed:
probe_failed:
	return NULL;
}

/*
 * Testcase Reporter
 */

typedef enum testcase_state__ testcase_state_t__;
enum testcase_state__ {
	CONSOLE_TESTCASE_PENDING,
	CONSOLE_TESTCASE_RUNNING,
	CONSOLE_TESTCASE_COMPLETED,
};

typedef struct testcase_reporter__ testcase_reporter_t__;
struct testcase_reporter__ {
	ctest_testcase_reporter_t base;

	FILE *fp;
	ctest_testcase_t *testcase;
	testcase_state_t__ state;
};

static testcase_reporter_t__ *upcast_testcase_reporter__(ctest_testcase_reporter_t *reporter)
{
	return containerof(reporter, testcase_reporter_t__, base);
}

static void testcase_repoter_report_failure__(testcase_reporter_t__ *reporter, ctest_failure_t *failure)
{
	const ctest_location_t *const location = failure->location;
	const ctest_stacktrace_t *const stacktrace = failure->stacktrace;
	if (location != NULL) {
		fprintf(reporter->fp, "Location: %s:%d\n", location->filename, location->line);
	}

	fprintf(reporter->fp, "Reason:\n");
	wrap_output__(reporter->fp, "    ", failure->description);
	if (stacktrace != NULL && stacktrace->length > 0) {
		size_t i;
		fprintf(reporter->fp, "Stacktrace:\n");
		for (i = 0; i < stacktrace->length; ++i) {
			const ctest_stackframe_t *const stackframe = stacktrace->frames + i;
			fprintf(reporter->fp, "      - %p", stackframe->addr);
			if (stackframe->filename != NULL) {
				fprintf(reporter->fp, " %s", stackframe->filename);
				if (stackframe->line > 0)
					fprintf(reporter->fp, ":%d", stackframe->line);
			}
			fprintf(reporter->fp, "\n");
		}
	}
}

static void testcase_reporter_report_output__(testcase_reporter_t__ *reporter, const ctest_output_t *output)
{
	if (output->length > 0) {
		/* TODO: Handle binary output */
		fprintf(reporter->fp, "Output:\n");
		wrap_output__(reporter->fp, "    ", output->data);
	}
}

CTEST_ALL_NONNULL_ARGS__
static void testcase_reporter_op_start__(ctest_testcase_reporter_t *ctest_reporter)
{
	testcase_reporter_t__ *const reporter = upcast_testcase_reporter__(ctest_reporter);

	if (reporter->state != CONSOLE_TESTCASE_PENDING)
		return;

	reporter->state = CONSOLE_TESTCASE_RUNNING;
}

CTEST_ALL_NONNULL_ARGS__
static void testcase_reporter_op_complete__(ctest_testcase_reporter_t *ctest_reporter, ctest_result_t *result)
{
	testcase_reporter_t__ *const reporter = upcast_testcase_reporter__(ctest_reporter);
	ctest_testcase_t *const testcase = reporter->testcase;
	ctest_test_t *const test = ctest_testcase_get_test(testcase);
	ctest_testsuite_t *const testsuite = ctest_test_get_testsuite(test);
	ctest_failure_t *const failure = result->failure;
	bool show_output = false;

	if (reporter->state != CONSOLE_TESTCASE_RUNNING)
		return;

	fprintf(reporter->fp, "%s:%s ... ", ctest_testsuite_get_name(testsuite), ctest_testcase_get_name(testcase));
	reporter->state = CONSOLE_TESTCASE_COMPLETED;

	switch(result->type) {
	case CTEST_RESULT_PASS:
		fprintf(reporter->fp, "OK\n");
		goto done;
	case CTEST_RESULT_FAIL:
		{
			const ctest_stage_t stage = failure != NULL ? failure->stage : CTEST_STAGE_EXECUTION;
			switch (stage) {
			case CTEST_STAGE_SETUP:
				fprintf(reporter->fp, "SETUP FAILED\n");
				goto fail;
			case CTEST_STAGE_EXECUTION:
				fprintf(reporter->fp, "FAILED\n");
				goto fail;
			case CTEST_STAGE_TEARDOWN:
				fprintf(reporter->fp, "TEARDOWN FAILED\n");
				goto fail;
			}
			fprintf(reporter->fp, "FAILED (unknown stage %d)\n", stage);
			goto fail;
		}
	case CTEST_RESULT_SKIPPED:
		fprintf(reporter->fp, "SKIPPED\n");
		goto done;
	case CTEST_RESULT_ERROR:
		fprintf(reporter->fp, "INTERNAL ERROR\n");
		goto fail;
	}

	fprintf(reporter->fp, "INTERNAL_FAILURE (unknown result: %d)\n", result->type);
fail:
	if (failure != NULL)
		 testcase_repoter_report_failure__(reporter, failure);
	show_output = true;
done:
	if (show_output && result->output != NULL)
		testcase_reporter_report_output__(reporter, result->output);
	ctest_result_destroy(result);

	/* Flush immediately, so there is no data that *could* be flushed
	 * by the runner, possibly after a fork. */
	fflush(reporter->fp);
}

CTEST_ALL_NONNULL_ARGS__
static void testcase_reporter_op_destroy__(ctest_testcase_reporter_t *ctest_reporter)
{
	testcase_reporter_t__ *const reporter = upcast_testcase_reporter__(ctest_reporter);
	memset(reporter, 0, sizeof(*reporter));
	(void)free(reporter);
}

CTEST_ALL_NONNULL_ARGS__
static testcase_reporter_t__ *testcase_reporter_create__(FILE *fp, ctest_testcase_t *testcase)
{
	static ctest_testcase_reporter_ops_t ops = {
		&testcase_reporter_op_start__,
		&testcase_reporter_op_complete__,
		&testcase_reporter_op_destroy__,
	};

	testcase_reporter_t__ *reporter;
	if ((reporter = calloc(1, sizeof(*reporter))) == NULL)
		goto alloc_reporter_failed;

	reporter->base.ops = &ops;
	reporter->fp = fp;
	reporter->testcase = testcase;
	reporter->state = CONSOLE_TESTCASE_PENDING;
	return reporter;

alloc_reporter_failed:
	return NULL;
}


/*
 * Test Reporter
 */

typedef struct test_reporter__ test_reporter_t__;
struct test_reporter__ {
	ctest_test_reporter_t base;

	FILE *fp;
	ctest_test_t *test;
};

static test_reporter_t__ *upcast_test_reporter__(ctest_test_reporter_t *reporter)
{
	return containerof(reporter, test_reporter_t__, base);
}


CTEST_ALL_NONNULL_ARGS__
static ctest_testcase_reporter_t *test_reporter_op_report_testcase__(ctest_test_reporter_t *ctest_reporter, ctest_testcase_t *testcase)
{
	test_reporter_t__ *const reporter = upcast_test_reporter__(ctest_reporter);
	testcase_reporter_t__ *testcase_reporter;

	ctest_test_t *const test = ctest_testcase_get_test(testcase);
	if (test != reporter->test) {
		errno = EINVAL;
		goto invalid_parameters;
	}

	if ((testcase_reporter = testcase_reporter_create__(reporter->fp, testcase)) == NULL)
		goto testcase_reporter_creation_failed;

	return &testcase_reporter->base;

testcase_reporter_creation_failed:
invalid_parameters:
	return NULL;
}

CTEST_ALL_NONNULL_ARGS__
static void test_reporter_op_destroy__(ctest_test_reporter_t *ctest_reporter)
{
	test_reporter_t__ *const reporter = upcast_test_reporter__(ctest_reporter);
	memset(reporter, 0, sizeof(*reporter));
	(void)free(reporter);
}

CTEST_ALL_NONNULL_ARGS__
static test_reporter_t__ *test_reporter_create__(FILE *fp, ctest_test_t *test)
{
	static ctest_test_reporter_ops_t ops = {
		&test_reporter_op_report_testcase__,
		&test_reporter_op_destroy__,
	};

	test_reporter_t__ *reporter;
	if ((reporter = calloc(1, sizeof(*reporter))) == NULL)
		goto alloc_reporter_failed;

	reporter->base.ops = &ops;
	reporter->fp = fp;
	reporter->test = test;
	return reporter;

alloc_reporter_failed:
	return NULL;
}

/*
 * Testsuite Reporter
 */

typedef struct testsuite_reporter__ testsuite_reporter_t__;
struct testsuite_reporter__ {
	ctest_testsuite_reporter_t base;

	FILE *fp;
	ctest_testsuite_t *testsuite;
};

static testsuite_reporter_t__ *upcast_testsuite_reporter__(ctest_testsuite_reporter_t *reporter)
{
	return containerof(reporter, testsuite_reporter_t__, base);
}

CTEST_ALL_NONNULL_ARGS__
static ctest_test_reporter_t *testsuite_reporter_op_report_test__(ctest_testsuite_reporter_t *ctest_reporter, ctest_test_t *test)
{
	testsuite_reporter_t__ *const reporter = upcast_testsuite_reporter__(ctest_reporter);
	test_reporter_t__ *test_reporter;

	if (ctest_test_get_testsuite(test) != reporter->testsuite) {
		errno = EINVAL;
		goto invalid_parameters;
	}

	if ((test_reporter = test_reporter_create__(reporter->fp, test)) == NULL)
		goto test_reporter_creation_failed;

	return &test_reporter->base;

test_reporter_creation_failed:
invalid_parameters:
	return NULL;
}

CTEST_ALL_NONNULL_ARGS__
static void testsuite_reporter_op_destroy__(ctest_testsuite_reporter_t *ctest_reporter)
{
	testsuite_reporter_t__ *const reporter = upcast_testsuite_reporter__(ctest_reporter);
	memset(reporter, 0, sizeof(*reporter));
	(void)free(reporter);
}

CTEST_ALL_NONNULL_ARGS__
static testsuite_reporter_t__ *testsuite_reporter_create__(FILE *fp, ctest_testsuite_t *testsuite)
{
	static ctest_testsuite_reporter_ops_t ops = {
		&testsuite_reporter_op_report_test__,
		&testsuite_reporter_op_destroy__,
	};

	testsuite_reporter_t__ *reporter;
	if ((reporter = calloc(1, sizeof(*reporter))) == NULL)
		goto alloc_reporter_failed;

	reporter->base.ops = &ops;
	reporter->fp = fp;
	reporter->testsuite = testsuite;
	return reporter;

alloc_reporter_failed:
	return NULL;
}

/*
 * Reporter
 */

typedef struct reporter__ reporter_t__;
struct reporter__ {
	ctest_reporter_t base;
	FILE *fp;
};

static reporter_t__ *upcast_reporter__(ctest_reporter_t *reporter)
{
	return containerof(reporter, reporter_t__, base);
}

CTEST_ALL_NONNULL_ARGS__
static ctest_testsuite_reporter_t *reporter_op_report_testsuite__(ctest_reporter_t *ctest_reporter, ctest_testsuite_t *testsuite) {
	reporter_t__ *const reporter = upcast_reporter__(ctest_reporter);
	testsuite_reporter_t__ *testsuite_reporter;

	if ((testsuite_reporter = testsuite_reporter_create__(reporter->fp, testsuite)) == NULL)
		goto create_failed;

	return &testsuite_reporter->base;

create_failed:
	return NULL;
}

CTEST_ALL_NONNULL_ARGS__
static void reporter_op_destroy__(ctest_reporter_t *ctest_reporter) {
	reporter_t__ *const reporter = upcast_reporter__(ctest_reporter);
	FILE *fp = reporter->fp;
	memset(reporter, 0, sizeof(*reporter));
	(void)free(reporter);
	(void)fclose(fp);
}

extern ctest_reporter_t *ctest_create_console_reporter(void) {
	static ctest_reporter_ops_t ops = {
		&reporter_op_report_testsuite__,
		&reporter_op_destroy__,
	};
	FILE *fp;

	reporter_t__ *reporter;
	if ((reporter = calloc(1, sizeof(*reporter))) == NULL)
		goto alloc_reporter_failed;
	/* Dup stdout, so it won't be affected by redirection that could happen
	 * in the runner. */
	if ((fp = dup__(stdout)) == NULL) {
		fprintf(stderr, "dup__ failed\n");
		goto dup_failed;
	}

	reporter->base.ops = &ops;
	reporter->fp = fp;
	return &reporter->base;

dup_failed:
	(void)free(reporter);
alloc_reporter_failed:
	return NULL;
}
