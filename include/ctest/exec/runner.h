#ifndef CTEST__EXEC__RUNNER_H__INCLUDED__
#define CTEST__EXEC__RUNNER_H__INCLUDED__

#include <stddef.h>

#include <ctest/_annotations.h>
#include <ctest/exec/reporter.h>
#include <ctest/exec/suite.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A test runner.
 *
 * A test runner is responsible for executing the tests within one or more test
 * suites. The details of the execution of tests is dependent on the runner
 * implementation (e.g., execution may be serial or parallel, with address space
 * isolation or without).
 *
 * Each test case will be executed and progress will be sent to the provided
 * reporter (which is responsible for reporting the results of the execution).
 */
typedef struct ctest_runner ctest_runner_t;
typedef const struct ctest_runner_ops ctest_runner_ops_t;
struct ctest_runner_ops {
	CTEST_ALL_NONNULL_ARGS__
	int (*run_testsuites)(ctest_runner_t *, ctest_reporter_t *, ctest_testsuite_t *const*, size_t);

	CTEST_ALL_NONNULL_ARGS__
	int (*run_tests)(ctest_runner_t *, ctest_reporter_t *, ctest_test_t *const*, size_t);

	CTEST_ALL_NONNULL_ARGS__
	int (*run_testcases)(ctest_runner_t *, ctest_reporter_t *, ctest_testcase_t *const*, size_t);

	CTEST_ALL_NONNULL_ARGS__
	void (*destroy)(ctest_runner_t *);
};
struct ctest_runner {
	ctest_runner_ops_t *ops;
};

/**
 * Run the test cases associated with the test suites defined by
 * <code>testsuites</code>.
 *
 * @param runner          The runner with which to run the test cases contained
 *                        within <code>testsuites</code>.
 * @param reporter        The reporter to which to report the results of the
 *                        test cases executed.
 * @param testsuites      The test suites to run.
 * @param testsuite_count The number of elements of <code>testsuites</code>.
 *
 * @return The number of test cases that failed or a negative number of there
 *         was a failure while attempting to run test cases. If zero is
 *         returned, all test cases passed or were skipped.
 */
CTEST_ALL_NONNULL_ARGS__
static inline int ctest_runner_run_testsuites(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_testsuite_t *const*testsuites, size_t testsuite_count)
{
	return (*runner->ops->run_testsuites)(runner, reporter, testsuites, testsuite_count);
}

/**
 * Run the test cases associated with the tests defined by <code>tests</code>.
 *
 * @param runner     The runner with which to run the test cases contained
 *                   within <code>tests</code>.
 * @param reporter   The reporter to which to report the results of the test
 *                   cases executed.
 * @param tests      The tests to run.
 * @param test_count The number of elements of <code>tests</code>.
 *
 * @return The number of test cases that failed or a negative number of there
 *         was a failure while attempting to run test cases. If zero is
 *         returned, all test cases passed or were skipped.
 */
CTEST_ALL_NONNULL_ARGS__
static inline int ctest_runner_run_tests(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_test_t *const*tests, size_t test_count)
{
	return (*runner->ops->run_tests)(runner, reporter, tests, test_count);
}

/**
 * Run the test cases defined by <code>testcases</code>.
 *
 * @param runner         The runner with which to run the test cases contained
 *                       within <code>testcases</code>.
 * @param reporter       The reporter to which to report the results of the
 *                       test cases executed.
 * @param testcases      The test cases to run.
 * @param testcase_count The number of elements of <code>testcases</code>.
 *
 * @return The number of test cases that failed or a negative number of there
 *         was a failure while attempting to run test cases. If zero is
 *         returned, all test cases passed or were skipped.
 */
CTEST_ALL_NONNULL_ARGS__
static inline int ctest_runner_run_testcases(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_testcase_t *const*testcases, size_t testcase_count)
{
	return (*runner->ops->run_testcases)(runner, reporter, testcases, testcase_count);
}

/**
 * Destroy <code>runner</code>, freeing its resources.
 *
 * @param runner The runner to destroy.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void ctest_runner_destroy(ctest_runner_t *runner)
{
	return (*runner->ops->destroy)(runner);
}

#ifdef __cplusplus
}
#endif
#endif /* CTEST__EXEC__RUNNER_H__INCLUDED__ */
