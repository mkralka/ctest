#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <ctest/_annotations.h>
#include <ctest/exec/runner.h>

#include "exec_events.h"
#include "output_reader.h"
#include "poll_handler.h"
#include "runner_utils.h"
#include "sig.h"
#include "utils.h"

/**
 * Coerce <code>type</code> into a valid <code>ctest_result_type_t</code>.
 *
 * If <code>type</code> does not represent a valid type,
 * <code>CTYPE_RESULT_ERROR</code> will be returned.
 *
 * @param type The value to coerce.
 *
 * @return A valid <code>ctest_result_type_t</code> representation of
 *         <code>type</code>.
 */
static ctest_result_type_t coerce_result_type__(int type)
{
	ctest_result_type_t result = (ctest_result_type_t)type;
	switch(result) {
	case CTEST_RESULT_PASS:
	case CTEST_RESULT_FAIL:
	case CTEST_RESULT_SKIPPED:
	case CTEST_RESULT_ERROR:
		return result;
	}

	return CTEST_RESULT_ERROR;
}

/**
 * Exit from the child process.
 *
 * @param result The result of the test.
 */
CTEST_NORETURN__
static void exit_child__(ctest_result_type_t result) {
	/* Ensure anything written by the child is flushed to the pipe before
	 * we exit. */
	fclose(stdout);
	fclose(stderr);
	fclose(stdin);
	exit(result);
}

/*
 * Execution Hooks
 */

/**
 * <code>ctest_exec_hooks_t</code> implementation for capturing execution
 * events in the child.
 *
 * After forking off a child process in which to run the tests, the results of
 * the test need to be communicated back with the parent. In the child, one of
 * these execution hooks is used to capture the execution events and write
 * then to the pipe back to the parent.
 */
typedef struct exec_hooks__ exec_hooks_t__;
struct exec_hooks__ {
	ctest_exec_hooks_t base;

	/**
	 * The stage in which the test is currently execution.
	 */
	ctest_stage_t stage;

	/**
	 * The writer to use to send information to the parent process. */
	exec_event_writer_t writer;
};

static inline exec_hooks_t__ *upcast_ctest_failure_hooks__(ctest_exec_hooks_t *hooks)
{
	return containerof(hooks, exec_hooks_t__, base);
}

static void exec_hooks_destroy__(exec_hooks_t__ *hooks);
static void exec_hooks_on_signal__(int signum, void *cookie)
{
	exec_hooks_t__ *const hooks = cookie;
	ctest_failure_t failure;
	char description[128];

	memset(&failure, 0, sizeof(failure));
	failure.stage = hooks->stage;
	failure.description = description;
	snprintf(description, sizeof(description), "Caught unexpected signal: %d\n", signum);

	exec_event_writer_on_failure(&hooks->writer, &failure);
	exec_hooks_destroy__(hooks);
	exit_child__(CTEST_RESULT_FAIL);
}

CTEST_NONNULL_ARGS__(1) CTEST_NORETURN__
static void exec_hooks_on_short_circuit__(ctest_exec_hooks_t *ctest_hooks, ctest_result_type_t result_type, ctest_failure_t *failure)
{
	exec_hooks_t__ *const hooks = upcast_ctest_failure_hooks__(ctest_hooks);

	if (failure != NULL) {
		exec_event_writer_on_failure(&hooks->writer, failure);
		ctest_failure_destroy(failure);
	}
	exec_hooks_destroy__(hooks);
	exit_child__(result_type);
}

static void exec_hooks_op_on_stage_change__(ctest_exec_hooks_t *ctest_hooks, ctest_stage_t stage)
{
	exec_hooks_t__ *const hooks = upcast_ctest_failure_hooks__(ctest_hooks);
	hooks->stage = stage;
	exec_event_writer_on_stage_change(&hooks->writer, stage);
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

static void exec_hooks_init__(exec_hooks_t__ *hooks, int fd)
{
	static ctest_exec_hooks_ops_t ops = {
		&exec_hooks_op_on_stage_change__,
		&exec_hooks_op_on_skip__,
		&exec_hooks_op_on_failure__,
	};

	hooks->base.ops = &ops;
	hooks->stage = CTEST_STAGE_SETUP;
	exec_event_writer_init(&hooks->writer, fd);
}

static void exec_hooks_destroy__(exec_hooks_t__ *hooks)
{
	exec_event_writer_destroy(&hooks->writer);
	memset(hooks, 0, sizeof(*hooks));
}

/*
 * Execution Event Consumer
 */

/**
 * <code>exec_event_consumer_t</code> implementation that consumes execution
 * events from the child.
 *
 * After forking off a child process in which to run the tests, the child will
 * write execution events to the parent (using an
 * <code>exec_event_writer_t</code>. Using an <code>exec_event_reader_t</code>,
 * the parent reads the events to be consumed by a
 * <code>exec_event_consumer_t</code>. <code>child_event_consumer_t__</code>
 * receives these events in the parent and processes them.
 */
typedef struct child_event_consumer__ child_event_consumer_t__;
struct child_event_consumer__ {
	exec_event_consumer_t base;
	ctest_stage_t stage;
	ctest_failure_t *last_failure;
};

static inline child_event_consumer_t__ *upcast_child_event_consumer__(exec_event_consumer_t *consumer)
{
	return containerof(consumer, child_event_consumer_t__, base);
}

static void child_event_consumer_op_on_stage_change__(exec_event_consumer_t *exec_event_consumer, ctest_stage_t stage)
{
	child_event_consumer_t__ *const consumer = upcast_child_event_consumer__(exec_event_consumer);
	consumer->stage = stage;
}

static void child_event_consumer_op_on_failure__(exec_event_consumer_t *exec_event_consumer, ctest_failure_t *failure)
{
	child_event_consumer_t__ *const consumer = upcast_child_event_consumer__(exec_event_consumer);
	if (consumer->last_failure != NULL)
		ctest_failure_destroy(consumer->last_failure);
	consumer->last_failure = failure;
}

static void child_event_consumer_init__(child_event_consumer_t__ *consumer)
{
	static exec_event_consumer_ops_t ops = {
		&child_event_consumer_op_on_stage_change__,
		&child_event_consumer_op_on_failure__,
	};

	consumer->base.ops = &ops;
	consumer->stage = CTEST_STAGE_SETUP;
	consumer->last_failure = NULL;
}

static void child_event_consumer_destroy__(child_event_consumer_t__ *consumer)
{
	if (consumer->last_failure != NULL) {
		ctest_failure_destroy(consumer->last_failure);
		consumer->last_failure = NULL;
	}
	memset(consumer, 0, sizeof(*consumer));
}

/*
 * Runner
 */

/**
 * A <code>ctest_runner_t</code> implementation that runs all tests isolated
 * in its own address space (by forking off and running the tests in a child).
 */
typedef struct forking_runner__ forking_runner_t__;
struct forking_runner__ {
	ctest_runner_t base;
};

static forking_runner_t__ *upcast_from_ctest_runner__(ctest_runner_t *runner)
{
	return containerof(runner, forking_runner_t__, base);
}

/**
 * Wait for the child process to complete execution of the unit test, capturing
 * the output and execution events it generates.
 *
 * @param result    The <code>ctest_result_t</code> object to update with the
 *                  results from running the test.
 * @param child     The PID of the child for which is being waited.
 * @param hooks_fd  The file descriptor of the pipe from which to read to
 *                  receive execution events from the child process.
 *                  This will be closed before returning.
 * @param output_fd The file descriptor of the pipe from which to read to
 *                  receive the output of the child process (stdout and stderr).
 *                  This will be closed before returning.
 *
 * @return Zero on success, non-zero on failure.
 */
static int wait_for_child__(ctest_result_t *result, pid_t child, int hooks_fd, int output_fd)
{
	child_event_consumer_t__ child_event_consumer;
	exec_event_reader_t child_event_reader;
	output_reader_t child_output_reader;

	struct {
		const char *name;
		int fd;
		poll_handler_t *handler;
	} poll_handlers[] = {
		{ "execution hooks", hooks_fd,  &child_event_reader.poll_handler_base },
		{ "output",          output_fd, &child_output_reader.poll_handler_base },
	};

	int child_result;
	pid_t wait_result;

	int retval = -1;

	child_event_consumer_init__(&child_event_consumer);
	exec_event_reader_init(&child_event_reader, hooks_fd, &child_event_consumer.base);
	output_reader_init(&child_output_reader, output_fd);

	while (1) {
		struct pollfd pollfds[countof(poll_handlers)];
		size_t i;
		int rc;

		rc = 0;
		memset(pollfds, 0, sizeof(pollfds));
		for (i = 0; i < countof(poll_handlers); ++i) {
			pollfds[i].fd = poll_handlers[i].fd;
			pollfds[i].events = POLLIN;
			if (poll_handlers[i].fd >= 0)
				rc = 1;
		}
		/* If no files are still open, nothing left to consume. */
		if (!rc) goto wait_for_child;

		if ((rc = poll(pollfds, countof(poll_handlers), -1)) < 0) {
			ctest_failure_t *const failure = ctest_failure_create(child_event_consumer.stage, "poll of child data failed: %s", NULL, NULL, strerror(errno));
			retval = ctest_result_set_failure(result, CTEST_RESULT_ERROR, failure);
			goto read_failure;
		}

		for (i = 0; i < countof(poll_handlers); ++i) {
			int f_close = 0;
			if (pollfds[i].revents & POLLIN) {
				rc = poll_handler_on_data_available(poll_handlers[i].handler);
				if (rc < 0) {
					ctest_failure_t *const failure = ctest_failure_create(child_event_consumer.stage, "consumption of %s from child failed: %s", NULL, NULL, poll_handlers[i].name, strerror(errno));
					retval = ctest_result_set_failure(result, CTEST_RESULT_ERROR, failure);
					f_close = 1;
				} else if (rc == 0) {
					/* Pipe has been closed */
					f_close = 1;
				}
			} else if (pollfds[i].revents & POLLHUP) {
				/* Pipe has been closed and all the data has been drained; */
				f_close = 1;
			}

			if (f_close) {
				/* Make negative so poll() will ignore it. */
				poll_handlers[i].fd = -poll_handlers[i].fd;
				poll_handler_on_close(poll_handlers[i].handler);
			}
		}
	}

read_failure:
	/* Failed to read from the child; just kill it. */
	kill(child, SIGKILL);

wait_for_child:
	/* FIXME: Timeout waiting for the child, then forcible kill it. */
	wait_result = waitpid(child, &child_result, 0);
	if (wait_result < 0) {
		ctest_failure_t *const failure = ctest_failure_create(child_event_consumer.stage, "error waiting for child: %s", NULL, NULL, strerror(errno));
		retval = ctest_result_set_failure(result, CTEST_RESULT_ERROR, failure);
		goto done;
	} else if (wait_result != child) {
		ctest_failure_t *const failure = ctest_failure_create(child_event_consumer.stage, "unexpected pid waited: %ji (expecting %ji)", NULL, NULL, (intmax_t) wait_result, (intmax_t) child);
		retval = ctest_result_set_failure(result, CTEST_RESULT_ERROR, failure);
		goto done;
	}

	if (WIFEXITED(child_result)) {
		ctest_result_type_t result_type = coerce_result_type__(WEXITSTATUS(child_result));
		switch (result_type) {
		case CTEST_RESULT_PASS:
			ctest_result_set_failure(result, result_type, NULL);
			retval = 0;
			break;
		case CTEST_RESULT_FAIL:
		case CTEST_RESULT_ERROR:
		case CTEST_RESULT_SKIPPED:
			/* report a failure unless the test was skipped. */
			retval = result_type != CTEST_RESULT_SKIPPED;
			ctest_result_set_failure(result, result_type, child_event_consumer.last_failure);
			child_event_consumer.last_failure = NULL;
		}
	} else if (WIFSIGNALED(child_result)) {
		int signum = WTERMSIG(child_result);
		ctest_failure_t *const failure = ctest_failure_create(child_event_consumer.stage, "terminated by signal: %s (%d)", NULL, NULL, strsignal(signum), signum);
		retval = ctest_result_set_failure(result, CTEST_RESULT_ERROR, failure);
	} else {
		ctest_failure_t *const failure = ctest_failure_create(child_event_consumer.stage, "child exited with error: %#x", NULL, NULL, child_result);
		retval = ctest_result_set_failure(result, CTEST_RESULT_ERROR, failure);
		goto done;
	}

done:
	result->output = output_reader_build(&child_output_reader);

	exec_event_reader_destroy(&child_event_reader);
	child_event_consumer_destroy__(&child_event_consumer);
	output_reader_destroy(&child_output_reader);
	return retval;
}

static int runner_run_testcase__(ctest_runner_t *unused(runner), ctest_testcase_reporter_t *reporter, ctest_testcase_t *testcase)
{
	ctest_result_t *result;
	int retval = -1;
	int hooks_pipe[2];              /* Pipe for sending hooks notifications to parent. */
	int output_pipe[2];             /* Pipe for sending test output (stderr/stdout) to parent. */

	if ((result = ctest_result_create_empty()) == NULL)
		goto result_creation_failed;

	ctest_testcase_reporter_start(reporter);

	if (pipe(hooks_pipe) != 0) {
		ctest_failure_t *const failure = ctest_failure_create(CTEST_STAGE_SETUP, "unable to create result pipe: %s", NULL, NULL, strerror(errno));
		retval = ctest_result_set_failure(result, CTEST_RESULT_ERROR, failure);
		goto hooks_pipe_failed;
	}
	if (pipe(output_pipe) != 0) {
		ctest_failure_t *const failure = ctest_failure_create(CTEST_STAGE_SETUP, "unable to create output pipe: %s", NULL, NULL, strerror(errno));
		retval = ctest_result_set_failure(result, CTEST_RESULT_ERROR, failure);
		goto output_pipe_failed;
	}

	pid_t pid = fork();
	if (pid < 0) {
		ctest_failure_t *const failure = ctest_failure_create(CTEST_STAGE_SETUP, "unable to fork child process: %s", NULL, NULL, strerror(errno));
		retval = ctest_result_set_failure(result, CTEST_RESULT_ERROR, failure);
		goto fork_failed;
	} else if (pid != 0) {
		/* Parent */
		const int hooks_fd = hooks_pipe[0];
		const int output_fd = output_pipe[0];

		/* Close the write end of the pipe; this ensures we get notified
		 * when the child exist. */
		(void)close(hooks_pipe[1]);
		(void)close(output_pipe[1]);

		retval = wait_for_child__(result, pid, hooks_fd, output_fd);

		ctest_testcase_reporter_complete(reporter, result);
		return retval;
	} else {
		/* Child */
		exec_hooks_t__ exec_hooks;
		const int hooks_fd = hooks_pipe[1];
		const int output_fd = output_pipe[1];
		int stdin_new;

		/* Open up a replacement for stdin */
		stdin_new = open("/dev/null", O_RDONLY);

		/* Close the read end of the pipe; this ensures we get notified
		 * when the parent dies. */
		(void)close(hooks_pipe[0]);
		(void)close(output_pipe[0]);

		/* Redirect stdin/stderr/stdout. */
		fflush(stdout);
		fflush(stderr);
		(void)close(STDIN_FILENO);
		(void)close(STDOUT_FILENO);
		(void)close(STDERR_FILENO);
		dup2(stdin_new, STDIN_FILENO);
		dup2(output_fd, STDOUT_FILENO);
		dup2(output_fd, STDERR_FILENO);
		(void)close(stdin_new);
		(void)close(output_fd);

		exec_hooks_init__(&exec_hooks, hooks_fd);
		sigcapture__(&exec_hooks_on_signal__, &exec_hooks);
		ctest_testcase_execute(testcase, &exec_hooks.base);
		sigrestore__();

		exec_hooks_destroy__(&exec_hooks);
		exit_child__(CTEST_RESULT_PASS);
	}

fork_failed:
	(void)close(output_pipe[0]);
	(void)close(output_pipe[1]);
output_pipe_failed:
	(void)close(hooks_pipe[0]);
	(void)close(hooks_pipe[1]);
hooks_pipe_failed:
	ctest_testcase_reporter_complete(reporter, result);
result_creation_failed:
	return retval;
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
	forking_runner_t__ *const runner = upcast_from_ctest_runner__(ctest_runner);
	memset(runner, 0, sizeof(*runner));
	(void)free(runner);
}

ctest_runner_t *ctest_create_forking_runner(void)
{
	static ctest_runner_ops_t ops = {
		&runner_op_run_testsuites__,
		&runner_op_run_tests__,
		&runner_op_run_testcases__,
		&runner_op_destroy__
	};

	forking_runner_t__ *runner;

	if ((runner = calloc(1, sizeof(*runner))) == NULL)
		goto alloc_runner_failed;

	runner->base.ops = &ops;
	return &runner->base;

alloc_runner_failed:
	return NULL;
}
