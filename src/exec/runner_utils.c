#include <stdlib.h>
#include <string.h>

#include <ctest/_annotations.h>
#include <ctest/exec/reporter.h>
#include <ctest/exec/runner.h>
#include <ctest/exec/suite.h>

#include "runner_utils.h"

CTEST_ALL_NONNULL_ARGS__
static ctest_test_t *const* repartition_tests__(ctest_test_t *const*tests, size_t test_count)
{
	ctest_test_t **sorted_tests;
	size_t i_read_test, i_write_test;

	if ((sorted_tests = calloc(sizeof(*sorted_tests), test_count)) == NULL)
		return NULL;
	memcpy(sorted_tests, tests, sizeof(*sorted_tests) * test_count);

	/* A well-behaved client will supply test cases that are already been in
	 * test order and tests in suite order, so the sorting SHOULD be a
	 * no-op; a simple simple insertion sort will maintain ordering. */
	for (i_write_test = 0; i_write_test < test_count; ++i_write_test) {
		ctest_testsuite_t *const testsuite = ctest_test_get_testsuite(sorted_tests[i_write_test]);

		/* i_write_test is already in the correct position. Start
		 * looking at tests that follow it. */
		for (i_read_test = i_write_test + 1; i_read_test < test_count; ++i_read_test) {
			ctest_test_t *const read_test = sorted_tests[i_read_test];
			if (ctest_test_get_testsuite(read_test) != testsuite)
				continue;

			i_write_test += 1;
			if (i_write_test == i_read_test)
				continue;
			memmove(sorted_tests + i_write_test + 1, sorted_tests + i_write_test, (i_read_test - i_write_test) * sizeof(*sorted_tests));
			sorted_tests[i_write_test] = read_test;
		}
	}

	return sorted_tests;
}

CTEST_ALL_NONNULL_ARGS__
static ctest_testcase_t *const* repartition_testcases__(ctest_testcase_t *const*testcases, size_t testcase_count)
{
	ctest_testcase_t **sorted_testcases;
	size_t i_read_testcase, i_write_testcase;

	if ((sorted_testcases = calloc(sizeof(*sorted_testcases), testcase_count)) == NULL)
		return NULL;
	memcpy(sorted_testcases, testcases, sizeof(*sorted_testcases) * testcase_count);

	/* A well-behaved client will supply test cases that are already been in
	 * test order and tests in suite order, so the sorting SHOULD be a
	 * no-op; a simple simple insertion sort will maintain ordering.
	 *
	 * The main body of the loop works as follows:
	 *
	 * 1. Make the active test the first element in the list.
	 * 2. Move all test cases that share the same test as the active test
	 *    case to the forward of the list so that they are adjacent to the
	 *    active test.
	 * 3. Take the first unprocessed test that is different than the active
	 *    test but shares the same test suite as the active test. If there
	 *    is such a test, make it active and go to step 2.
	 * 4. Take the first unprocessed test. If there is such a test, make it
	 *    active and go to step 2.
	 *
	 * This will make sure that test cases associated with the same test
	 * will be contiguous and have their order preserved, while also
	 * ensuring that test cases associated with the same suite are
	 * contiguous and have their order preserved, except where necessary to
	 * keep test cases associated with the same test contiguous.
	 */
	for (i_write_testcase = 0; i_write_testcase < testcase_count; ) {
		/* All the test cases that belong to test suites that appear
		 * before i_write_testcase in sorted_testcases are all before
		 * i_write_testcase. The test case at position i_write_testcase
		 * belongs to a new test and test suite, so it should remain
		 * in its current position and test cases that belong to the
		 * same test should be moved into position immediately after
		 * i_write_testcase. */
		ctest_test_t *next_test = ctest_testcase_get_test(sorted_testcases[i_write_testcase]);
		i_write_testcase += 1;
		while (next_test != NULL) {
			ctest_test_t *const test = next_test;
			ctest_testsuite_t *const testsuite = ctest_test_get_testsuite(test);

			next_test = NULL;
			for (i_read_testcase = i_write_testcase; i_read_testcase < testcase_count; ++i_read_testcase) {
				ctest_testcase_t *const read_testcase = sorted_testcases[i_read_testcase];
				ctest_test_t *const read_test = ctest_testcase_get_test(read_testcase);
				if (read_test != test) {
					/* This test case belongs to a different
					 * test but the same suite. If this is
					 * the first, then the test should be
					 * the next one that appears in the
					 * output. */
					if (next_test == NULL && ctest_test_get_testsuite(read_test) == testsuite)
						next_test = read_test;
					continue;
				}

				/* This test case belongs to the test that is
				 * being moved forward. Move it forward. */
				if (i_write_testcase != i_read_testcase) {
					memmove(sorted_testcases + i_write_testcase + 1, sorted_testcases + i_write_testcase, (i_read_testcase - i_write_testcase) * sizeof(*sorted_testcases));
					sorted_testcases[i_write_testcase] = read_testcase;
				}
				i_write_testcase += 1;
			}
		};
	}

	return sorted_testcases;
}

/**
 * Run one or more test cases associated with a single test.
 *
 * @param runner         The runner to run each test case.
 * @param reporter       The reporter to which to report the results of running
 *                       each test case.
 * @param testcases      The test cases to run.
 * @param testcase_count The number of test cases in <tt>testcases</tt>.
 * @param run_testcase   The runner-specific callback for running an individual
 *                       test case.
 * @return The number of test cases that failed, or -1 if an error was
 *          encountered.
 */
CTEST_ALL_NONNULL_ARGS__
static int run_testcases_in_test__(ctest_runner_t *runner, ctest_test_reporter_t *reporter, ctest_testcase_t *const*testcases, size_t testcase_count, int (*run_testcase)(ctest_runner_t *, ctest_testcase_reporter_t *, ctest_testcase_t *))
{
	int result = 0;
	size_t i;
	for (i = 0; i < testcase_count; ++i) {
		ctest_testcase_t *const testcase = testcases[i];
		ctest_testcase_reporter_t *testcase_reporter;
		int rc;

		if ((testcase_reporter = ctest_test_reporter_report_testcase(reporter, testcase)) == NULL)
			return -1;

		rc = (*run_testcase)(runner, testcase_reporter, testcase);
		ctest_testcase_reporter_destroy(testcase_reporter);

		if (rc < 0)
			return -1;
		result += (rc > 0);
	}
	return result;
}

/**
 * Run all of the test cases associated with the tests from a single test suite.
 *
 * @param runner       The runner to run each test case.
 * @param reporter     The reporter to which to report the results of running
 *                     each test case.
 * @param tests        The tests to run.
 * @param test_count   The number of tests in <tt>tests</tt>.
 * @param run_testcase The runner-specific callback for running an individual
 *                     test case.
 * @return The number of test cases that failed, or -1 if an error was
 *         encountered.
 */
CTEST_ALL_NONNULL_ARGS__
static int run_tests_in_testsuite__(ctest_runner_t *runner, ctest_testsuite_reporter_t *reporter, ctest_test_t *const*tests, size_t test_count, int (*run_testcase)(ctest_runner_t *, ctest_testcase_reporter_t *, ctest_testcase_t *))
{
	int result = 0;
	size_t i;
	for (i = 0; i < test_count; ++i) {
		ctest_test_t *const test = tests[i];
		ctest_test_reporter_t *test_reporter;
		int rc;

		if ((test_reporter = ctest_testsuite_reporter_report_test(reporter, test)) == NULL)
			return -1;

		rc = run_testcases_in_test__(runner, test_reporter, ctest_test_get_testcases(test), ctest_test_get_testcase_count(test), run_testcase);
		ctest_test_reporter_destroy(test_reporter);

		if (rc < 0)
			return rc;
		result += rc;
	}
	return result;
}

CTEST_ALL_NONNULL_ARGS__
int runner_run_testcases(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_testcase_t *const*testcases, size_t testcase_count, int (*run_testcase)(ctest_runner_t *, ctest_testcase_reporter_t *, ctest_testcase_t *))
{
	ctest_testsuite_t *testsuite;
	ctest_testsuite_reporter_t *testsuite_reporter;
	size_t i_testcase;
	int result = -1;

	if (testcase_count == 0)
		return 0;

	if ((testcases = repartition_testcases__(testcases, testcase_count)) == NULL)
		goto repartition_testcases_failed;

	result = 0;
	testsuite = NULL;
	testsuite_reporter = NULL;
	for (i_testcase = 0; i_testcase < testcase_count; ) {
		ctest_test_t *const test = ctest_testcase_get_test(testcases[i_testcase]);
		ctest_testsuite_t *const this_testsuite = ctest_test_get_testsuite(test);
		ctest_test_reporter_t *test_reporter;
		const size_t i_first_testcase = i_testcase;
		int rc;

		if (this_testsuite != testsuite) {
			if (testsuite_reporter != NULL)
				ctest_testsuite_reporter_destroy(testsuite_reporter);
			testsuite_reporter = ctest_reporter_report_testsuite(reporter, this_testsuite);
			if (testsuite_reporter == NULL) {
				result = -1;
				break;
			}
			testsuite = this_testsuite;
		}

		for (i_testcase += 1; i_testcase < testcase_count; ++i_testcase) {
			if (ctest_testcase_get_test(testcases[i_testcase]) != test)
				break;
		}

		test_reporter = ctest_testsuite_reporter_report_test(testsuite_reporter, test);
		if (test_reporter == NULL) {
			result = -1;
			break;
		}

		rc = run_testcases_in_test__(runner, test_reporter, testcases + i_first_testcase, i_testcase - i_first_testcase, run_testcase);
		ctest_test_reporter_destroy(test_reporter);
		if (rc < 0) {
			result = -1;
			break;
		}
		result += rc;
	}

	if (testsuite_reporter != NULL)
		ctest_testsuite_reporter_destroy(testsuite_reporter);
	(void)free((void*)testcases);
repartition_testcases_failed:
	return result;
}

CTEST_ALL_NONNULL_ARGS__
int runner_run_tests(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_test_t *const*tests, size_t test_count, int (*run_testcase)(ctest_runner_t *, ctest_testcase_reporter_t *, ctest_testcase_t *))
{
	size_t i_test;
	int result = -1;

	if (test_count == 0)
		return 0;

	if ((tests = repartition_tests__(tests, test_count)) == NULL)
		goto repartition_tests_failed;

	result = 0;
	for (i_test = 0; i_test < test_count; ) {
		ctest_testsuite_t *const testsuite = ctest_test_get_testsuite(tests[i_test]);
		ctest_testsuite_reporter_t *testsuite_reporter;
		const size_t i_first_test = i_test;
		int rc;

		for (i_test += 1; i_test < test_count; ++i_test) {
			if (ctest_test_get_testsuite(tests[i_test]) != testsuite)
				break;
		}

		testsuite_reporter = ctest_reporter_report_testsuite(reporter, testsuite);
		if (testsuite_reporter == NULL) {
			result = -1;
			break;
		}

		rc = run_tests_in_testsuite__(runner, testsuite_reporter, tests + i_first_test, i_test - i_first_test, run_testcase);
		ctest_testsuite_reporter_destroy(testsuite_reporter);
		if (rc < 0) {
			result = -1;
			break;
		}
		result += rc;
	}

	(void)free((void*)tests);
repartition_tests_failed:
	return result;
}

int runner_run_testsuites(ctest_runner_t *runner, ctest_reporter_t *reporter, ctest_testsuite_t *const*testsuites, size_t testsuite_count, int (*run_testcase)(ctest_runner_t *, ctest_testcase_reporter_t *, ctest_testcase_t *))
{
	int result = 0;
	size_t i;
	for (i = 0; i < testsuite_count; ++i) {
		ctest_testsuite_t *const testsuite = testsuites[i];
		ctest_testsuite_reporter_t *testsuite_reporter;
		int rc;

		if ((testsuite_reporter = ctest_reporter_report_testsuite(reporter, testsuite)) == NULL)
			return -1;

		rc = run_tests_in_testsuite__(runner, testsuite_reporter, ctest_testsuite_get_tests(testsuite), ctest_testsuite_get_test_count(testsuite), run_testcase);
		ctest_testsuite_reporter_destroy(testsuite_reporter);

		if (rc < 0)
			return rc;
		result += rc;
	}
	return result;
}
