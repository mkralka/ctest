#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ctest/_annotations.h>
#include <ctest/exec/stage.h>
#include <ctest/exec/result.h>
#include "runner_utils.h"
#include "sig.h"
#include "utils.h"


/* Return values from setjmp indicating how the test completed. */
#define RESULT_TYPE_NORMAL__		1
#define RESULT_TYPE_SIGNAL__		2
#define RESULT_TYPE_ERRNO__		3

/**
 * Determine the system's temporary directory.
 *
 * @return The directory in which temporary files should be created.
 */
static const char *get_tmpdir(void)
{
	static const char * const names[] = {
		"TMPDIR",
		"TEMP",
		"TMP",
		"TEMPDIR",
	};
	size_t i;

	for (i = 0; i < countof(names); ++i) {
		const char *const name = names[i];
		char *value = getenv(name);
		if (value != NULL)
			return value;
	}

	return "/tmp";
}

/**
 * Create a temporary suitable for reading/writing that is automatically
 * deleted when closed.
 *
 * @return The file descriptor of a file that is automatically deleted when
 *         closed, or -1 on error.
 */
static int opentemp(void)
{
	static const char template[] = "ctest_XXXXXXXX";
	const char *tmpdir;
	size_t tmpfile_len;
	char *tmpfile;
	int fd = -1;

	if ((tmpdir = get_tmpdir()) == NULL)
		goto get_tmpdir_failed;

	tmpfile_len = strlen(tmpdir) + 1 + strlen(template) + 1;
	if ((tmpfile = malloc(tmpfile_len)) == NULL)
		goto tmpfile_alloc_failed;

	snprintf(tmpfile, tmpfile_len, "%s/%s", tmpdir, template);

	if ((fd = mkstemp(tmpfile)) < 0)
		goto mkstemp_failed;

	/* Unlink the file, so that it's automatically deleted. */
	unlink(tmpfile);

mkstemp_failed:
	(void)free(tmpfile);
tmpfile_alloc_failed:
get_tmpdir_failed:
	return fd;
}

static ctest_output_t *read_output__(int fd)
{
	ctest_output_t *result = NULL;
	off_t offset;
	int read_len;

	if ((offset = lseek(fd, 0, SEEK_END)) < 0)
		goto seek_failed;

	if ((result = ctest_output_create(offset+1)) == NULL)
		goto create_failed;

	lseek(fd, 0, SEEK_SET);
	if ((read_len = read(fd, result->data, (size_t)offset)) < 0)
		read_len = 0;
	result->length = read_len;

create_failed:
seek_failed:
	return result;
}

/*
 * Failure Hooks
 */

typedef struct exec_hooks__ exec_hooks_t__;
struct exec_hooks__ {
	ctest_exec_hooks_t base;

	sigjmp_buf env;
	ctest_result_t *result;
	int error;              /* signal number or errno value */
	ctest_stage_t stage;
};

static inline exec_hooks_t__ *upcast_ctest_exec_hooks__(ctest_exec_hooks_t *hooks)
{
	return containerof(hooks, exec_hooks_t__, base);
}

CTEST_NONNULL_ARGS__(1) CTEST_NORETURN__
static void exec_hooks_on_short_circuit__(ctest_exec_hooks_t *ctest_hooks, ctest_result_type_t type, ctest_failure_t *failure)
{
	exec_hooks_t__ *const hooks = upcast_ctest_exec_hooks__(ctest_hooks);

	ctest_result_set_failure(hooks->result, type, failure);
	siglongjmp(hooks->env, RESULT_TYPE_NORMAL__);
}

static void exec_hooks_op_on_stage_change__(ctest_exec_hooks_t *ctest_hooks, ctest_stage_t stage)
{
	exec_hooks_t__ *const hooks = upcast_ctest_exec_hooks__(ctest_hooks);
	hooks->stage = stage;
}

CTEST_NONNULL_ARGS__(1) CTEST_NORETURN__
static void exec_hooks_op_on_skip__(ctest_exec_hooks_t *hooks, ctest_failure_t *failure)
{
	exec_hooks_on_short_circuit__(hooks, CTEST_RESULT_SKIPPED, failure);
}

CTEST_NONNULL_ARGS__(1) CTEST_NORETURN__
static void exec_hooks_op_on_failure__(ctest_exec_hooks_t *hooks, ctest_failure_t *failure)
{
	exec_hooks_on_short_circuit__(hooks, CTEST_RESULT_FAIL, failure);
}

static void exec_hooks_init__(exec_hooks_t__ *hooks)
{
	static ctest_exec_hooks_ops_t ops = {
		&exec_hooks_op_on_stage_change__,
		&exec_hooks_op_on_skip__,
		&exec_hooks_op_on_failure__,
	};

	hooks->base.ops = &ops;
	hooks->result = ctest_result_create_empty();
	hooks->error = 0;
	hooks->stage = CTEST_STAGE_SETUP;
}

/*
 * Runner
 */

typedef struct direct_runner__ direct_runner_t__;
struct direct_runner__ {
	ctest_runner_t base;
};

static inline direct_runner_t__ *upcast_ctest_runner__(ctest_runner_t *runner)
{
	return containerof(runner, direct_runner_t__, base);
}

static void handle_signal__(int signum, void *cookie)
{
	exec_hooks_t__ *const hooks = cookie;
	hooks->error = signum;
	siglongjmp(hooks->env, RESULT_TYPE_SIGNAL__);
}

CTEST_ALL_NONNULL_ARGS__
static int runner_run_testcase__(ctest_runner_t *unused(runner), ctest_testcase_reporter_t *reporter, ctest_testcase_t *testcase)
{
	exec_hooks_t__ exec_hooks;
	int rc, result = 1;
	int stdin_saved, stdout_saved, stderr_saved, stdin_new, stdout_new;

	exec_hooks_init__(&exec_hooks);

	/* Save existing stdin, stdout, stderr for redirection. */
	if ((stdin_saved = dup(STDIN_FILENO)) < 0)
		goto stdin_saved_failed;
	if ((stdout_saved = dup(STDOUT_FILENO)) < 0)
		goto stdout_saved_failed;
	if ((stderr_saved = dup(STDERR_FILENO)) < 0)
		goto stderr_saved_failed;

	/* Open up new fds for stdin/stdout/stderr */
	if ((stdin_new = open("/dev/null", O_RDONLY)) < 0)
		goto stdin_new_failed;
	if ((stdout_new = opentemp()) < 0)
		goto stdout_new_failed;

	switch (rc = sigsetjmp(exec_hooks.env, 1)) {
	case 0:
		/* return from setjmp */
		ctest_testcase_reporter_start(reporter);

		/* Redirect stdin/stdout/stderr */
		fflush(stdout);
		fflush(stderr);
		(void)close(STDIN_FILENO);
		(void)close(STDOUT_FILENO);
		(void)close(STDERR_FILENO);
		dup2(stdin_new, STDIN_FILENO);
		dup2(stdout_new, STDOUT_FILENO);
		dup2(stdout_new, STDERR_FILENO);

		sigcapture__(handle_signal__, &exec_hooks);
		ctest_testcase_execute(testcase, &exec_hooks.base);
		exec_hooks.result->type = CTEST_RESULT_PASS;
		result = 0;
		break;

	case RESULT_TYPE_NORMAL__:
		/* return from siglongjmp normally (result is filled in) */
		break;

	case RESULT_TYPE_SIGNAL__:
		/* return from siglongjmp due to caught signal. */
		{
			ctest_failure_t *const failure = ctest_failure_create(exec_hooks.stage, "Caught unexpected signal: %d", NULL, NULL, exec_hooks.error);
			/* FIXME: What if signal happens during setup/teardown? */
			ctest_result_set_failure(exec_hooks.result, CTEST_RESULT_FAIL, failure);
		}
		break;

	case RESULT_TYPE_ERRNO__:
		/* return from siglongjmp due to unforeseen error. */
		{
			ctest_failure_t *const failure = ctest_failure_create(exec_hooks.stage, "Unexpected error encountered: %s", NULL, NULL, strerror(exec_hooks.error));
			ctest_result_set_failure(exec_hooks.result, CTEST_RESULT_ERROR, failure);
		}
		break;

	default:
		/* return from siglongjmp for unknown reason */
		{
			ctest_failure_t *const failure = ctest_failure_create(exec_hooks.stage, "unexpected return from longjmp: %d", NULL, NULL, rc);
			ctest_result_set_failure(exec_hooks.result, CTEST_RESULT_ERROR, failure);
		}
		break;
	}
	sigrestore__();

	/* Undo redirection stdin/stdout/stderr */
	fflush(stdout);
	fflush(stderr);
	(void)close(STDIN_FILENO);
	(void)close(STDOUT_FILENO);
	(void)close(STDERR_FILENO);
	dup2(stdin_saved, STDIN_FILENO);
	dup2(stdout_saved, STDOUT_FILENO);
	dup2(stderr_saved, STDERR_FILENO);

	ctest_result_set_output(exec_hooks.result, read_output__(stdout_new));
	ctest_testcase_reporter_complete(reporter, exec_hooks.result);

	(void)close(stdout_new);
stdout_new_failed:
	(void)close(stdin_new);
stdin_new_failed:
	(void)close(stderr_saved);
stderr_saved_failed:
	(void)close(stdout_saved);
stdout_saved_failed:
	(void)close(stdin_saved);
stdin_saved_failed:
	return result;
}

CTEST_ALL_NONNULL_ARGS__
static int runner_op_run_testsuites__(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_testsuite_t *const* testsuites, size_t testsuite_count)
{
	return runner_run_testsuites(runner, reporter, testsuites, testsuite_count, &runner_run_testcase__);
}

CTEST_ALL_NONNULL_ARGS__
static int runner_op_run_tests__(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_test_t *const*tests, size_t test_count)
{
	return runner_run_tests(runner, reporter, tests, test_count, &runner_run_testcase__);
}

CTEST_ALL_NONNULL_ARGS__
static int runner_op_run_testcases__(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_testcase_t *const*testcases, size_t testcase_count)
{
	return runner_run_testcases(runner, reporter, testcases, testcase_count, &runner_run_testcase__);
}

CTEST_ALL_NONNULL_ARGS__
static void runner_op_destroy__(ctest_runner_t *ctest_runner)
{
	direct_runner_t__ *const runner = upcast_ctest_runner__(ctest_runner);
	memset(runner, 0, sizeof(*runner));
	(void)free(runner);
}

ctest_runner_t *ctest_create_direct_runner(void)
{
	static ctest_runner_ops_t ops = {
		&runner_op_run_testsuites__,
		&runner_op_run_tests__,
		&runner_op_run_testcases__,
		&runner_op_destroy__
	};

	direct_runner_t__ *runner;

	if ((runner = calloc(1, sizeof(*runner))) == NULL)
		goto alloc_runner_failed;

	runner->base.ops = &ops;
	return &runner->base;

alloc_runner_failed:
	return NULL;
}
