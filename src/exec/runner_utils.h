#ifndef PRIVATE__REPORTER_UTILS_H__INCLUDED__
#define PRIVATE__REPORTER_UTILS_H__INCLUDED__

#include <ctest/_annotations.h>
#include <ctest/exec/reporter.h>
#include <ctest/exec/runner.h>
#include <ctest/exec/suite.h>

/**
 * Run a collection of test cases associated with different tests.
 *
 * Run a collection of test cases that are associated with different tests by
 * grouping the test cases from the same tests together and tests cases from
 * the same suite together, running them in a group. As much as possible, the
 * ordering of the tests is preserved. The ordering of test cases within a
 * given test is preserved as is the ordering of tests and suites (based on the
 * first occurrence of a test case that belongs to the test or suite).
 *
 * @param runner          The runner in which the tests will be run.
 * @param reporter        The reporter to use to for reporting the results of
 *                        the tests.
 * @param testcases       The collection of test cases to partition.
 * @param testcases_count The length of the collection of tests cases to
 *                        partition.
 * @param run_testcase    A callback to invoke for each test case that
 *                        implements the runner-specific logic. The callback
 *                        should return 0 if the test succeeded, positive value
 *                        if the test failed, and a negative value if an error
 *                        was encountered while running the test.
 */
CTEST_ALL_NONNULL_ARGS__
int runner_run_testcases(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_testcase_t *const*testcases, size_t testcase_count, int (*run_testcase)(ctest_runner_t *, ctest_testcase_reporter_t *, ctest_testcase_t *));

/**
 * Run a collection of tests associated with different suites.
 *
 * Run a collection of tests associated with different suites by grouping the
 * tests from the same suite together and running them as a group. As much as
 * possible, the ordering of the tests is preserved. The ordering of tests
 * within a given suite is preserved as is the order of suites (based on the
 * first occurrence of a test that belongs to the suite).
 *
 * @param runner       The runner in which the tests will be run.
 * @param reporter     The reporter to use to for reporting the results of the
 *                     tests.
 * @param tests        The collection of tests to run.
 * @param test_count   The number of tests in the collection.
 * @param run_testcase A callback to invoke for each test case that implements
 *                     the runner-specific logic. The callback should return
 *                     0 if the test succeeded, positive value if the test
 *                     failed, and a negative value if an error was encountered
 *                     while running the test.
 * @return The number of test cases that failed, or a negative number if an
 *         error was encountered.
 */
CTEST_ALL_NONNULL_ARGS__
int runner_run_tests(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_test_t *const*tests, size_t test_count, int (*run_testcase)(ctest_runner_t *, ctest_testcase_reporter_t *, ctest_testcase_t *));

/**
 * Run a collection of test suites.
 *
 * Run all of the tests and test cases associated with a collection of test
 * suites.
 *
 * @param runner          The runner in which the tests will be run.
 * @param reporter        The reporter to use to for reporting the results of
 *                        the tests.
 * @param testsuites      The collection of tests to run.
 * @param testsuite_count The number of tests in the collection.
 * @param run_testcase    A callback to invoke for each test case that
 *                        implements the runner-specific logic. The callback
 *                        should return 0 if the test succeeded, positive value
 *                        if the test failed, and a negative value if an error
 *                        was encountered while running the test.
 * @return The number of test cases that failed, or a negative number if an
 *         error was encountered.
 */
CTEST_ALL_NONNULL_ARGS__
int runner_run_testsuites(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_testsuite_t *const*testsuites, size_t testsuite_count, int (*run_testcase)(ctest_runner_t *, ctest_testcase_reporter_t *, ctest_testcase_t *));

#endif /* PRIVATE__REPORTER_UTILS_H__INCLUDED__ */
