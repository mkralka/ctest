/**
 * A test reporter.
 *
 * A <em>test reporter</em> is responsible for reporting the results of running
 * tests from a test suite. Different reporters may handle the results
 * differently, for example:
 * <ul>
 * <li>A GUI may display the results in an collapsible tree view,</li>
 * <li>A CLI may print the results, organized by suite and test, in a human
 * readable format,</li>
 * <li>A build tool may write the results in a machine readable format to a
 * file.</li>
 * </ul>
 */
#ifndef CTEST__EXEC__REPORTER_H__INCLUDED__
#define CTEST__EXEC__REPORTER_H__INCLUDED__

#include <ctest/_annotations.h>
#include <ctest/exec/suite.h>
#include <ctest/exec/result.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A reporter for reporting the progress of an individual test case.
 */
typedef struct ctest_testcase_reporter ctest_testcase_reporter_t;
typedef const struct ctest_testcase_reporter_ops ctest_testcase_reporter_ops_t;
struct ctest_testcase_reporter_ops {
	CTEST_ALL_NONNULL_ARGS__
	void (*start)(ctest_testcase_reporter_t *);

	CTEST_ALL_NONNULL_ARGS__
	void (*complete)(ctest_testcase_reporter_t *, ctest_result_t *);

	CTEST_ALL_NONNULL_ARGS__
	void (*destroy)(ctest_testcase_reporter_t *);
};
struct ctest_testcase_reporter {
	ctest_testcase_reporter_ops_t *ops;
};

/**
 * Report a test case as started (running).
 *
 * @param reporter The reporter to which to report the started test case.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void ctest_testcase_reporter_start(ctest_testcase_reporter_t *reporter)
{
	return (*reporter->ops->start)(reporter);
}

/**
 * Report a test case as completed.
 *
 * @param reporter The reporter to which to report the started test case.
 * @param result The result of the test.
 *
 * @note
 * Ownership of the report is transferred to the reporter; the reporter is
 * responsible for destroying the result using @link ctest_result_destroy.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void ctest_testcase_reporter_complete(ctest_testcase_reporter_t *reporter, ctest_result_t *result)
{
	return (*reporter->ops->complete)(reporter, result);
}

/**
 * Destroy a test case reporter, freeing any resources it may have.
 *
 * If the test case has not yet completed, the test will be considered to have
 * been cancelled.
 *
 * @param reporter The test case reporter to destory.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void ctest_testcase_reporter_destroy(ctest_testcase_reporter_t *reporter)
{
	return (*reporter->ops->destroy)(reporter);
}

/**
 * A reporter for reporting the state of all test cases associated with
 * a test.
 *
 * Test cases are reported by created individual test case reporters
 * for each associated test case. The individual test case reporters are then
 * used to report the results of test cases are started and completed.
 */
typedef struct ctest_test_reporter ctest_test_reporter_t;
typedef const struct ctest_test_reporter_ops ctest_test_reporter_ops_t;
struct ctest_test_reporter_ops {
	CTEST_ALL_NONNULL_ARGS__
	ctest_testcase_reporter_t *(*report_testcase)(ctest_test_reporter_t *, ctest_testcase_t *);

	CTEST_ALL_NONNULL_ARGS__
	void (*destroy)(ctest_test_reporter_t *);
};
struct ctest_test_reporter {
	ctest_test_reporter_ops_t *ops;
};

/**
 * Create a reporter for an individual test case.
 *
 * @param reporter The test reporter from which to create the test case
 *                 reporter.
 * @param testcase The test case for which to create the test case reporter.
 *
 * @return A new test case reporter to use to report the progress of the test
 *         case or <code>NULL</code> if <code>testcase</code> does not belong to
 *         the test for which <code>reporter</code> was built or an error
 *         occurred building the test case reporter.
 */
CTEST_ALL_NONNULL_ARGS__
static inline ctest_testcase_reporter_t *ctest_test_reporter_report_testcase(ctest_test_reporter_t *reporter, ctest_testcase_t *testcase)
{
	return (*reporter->ops->report_testcase)(reporter, testcase);
}

/**
 * Destroy a test reporter, freeing any resources it may have.
 *
 * Any pending test cases will be abandoned and considered to have been
 * cancelled (the objects will still be valid, though). When results to these
 * abandoned test cases are reported (via @link
 * ctest_testcase_reporter_complete), the results will be ignored.
 *
 * @param reporter The test reporter to destroy.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void ctest_test_reporter_destroy(ctest_test_reporter_t *reporter)
{
	return (*reporter->ops->destroy)(reporter);
}

/**
 * A reporter for reporting the state of all tests associated with a test suite.
 *
 * Test are reported by creating individual test reporters for each associated
 * test. The individual test reporters are then used to report the results of
 * the tests cases as they are started and completed.
 */
typedef struct ctest_testsuite_reporter ctest_testsuite_reporter_t;
typedef const struct ctest_testsuite_reporter_ops ctest_testsuite_reporter_ops_t;
struct ctest_testsuite_reporter_ops {
	CTEST_ALL_NONNULL_ARGS__
	ctest_test_reporter_t *(*report_test)(ctest_testsuite_reporter_t *, ctest_test_t *);

	CTEST_ALL_NONNULL_ARGS__
	void (*destroy)(ctest_testsuite_reporter_t *);
};
struct ctest_testsuite_reporter {
	ctest_testsuite_reporter_ops_t *ops;
};

/**
 * Create a reporter for an individual test.
 *
 * Reporters can then be created for individual test cases using the returned
 * test reporter.
 *
 * @param reporter The test suite reporter from which to create the test
 *                 reporter.
 * @param test The test for which to create the test reporter.
 *
 * @return A new test reporter to use to report the progress of a test, or
 *         <code>NULL</code> if <code>test</code> does not belong to the test
 *         suite for which <code>reporter</code> was built or an error occurred
 *         building the test case reporter.
 */
CTEST_ALL_NONNULL_ARGS__
static inline ctest_test_reporter_t *ctest_testsuite_reporter_report_test(ctest_testsuite_reporter_t *reporter, ctest_test_t *test)
{
	return (*reporter->ops->report_test)(reporter, test);
}

/**
 * Destroy a test suite reporter, freeing any resources it may have.
 *
 * Any pending test cases will be abandoned and considered to have been
 * cancelled (the objects will still be valid, though). When results to those
 * abandoned tests are reported (via @link ctest_testcase_reporter_complete),
 * the results will be ignored. Any new test cases started (via @link
 * ctest_testcase_reporter_start) will also be ignored.
 *
 * @param reporter The reporter to destroy.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void ctest_testsuite_reporter_destroy(ctest_testsuite_reporter_t *reporter)
{
	return (*reporter->ops->destroy)(reporter);
}

/**
 * A reporter for reporting the execution of test cases associated with test
 * suites.
 *
 * This reporter is not used directly, but is used to create reporters for
 * individual suites, tests, and test cases. The details of these are specific
 * to the reporter implementation (e.g., GUI, CLI, etc.).
 */
typedef struct ctest_reporter ctest_reporter_t;
typedef const struct ctest_reporter_ops ctest_reporter_ops_t;
struct ctest_reporter_ops {
	CTEST_ALL_NONNULL_ARGS__
	ctest_testsuite_reporter_t *(*report_testsuite)(ctest_reporter_t *, ctest_testsuite_t *);

	CTEST_ALL_NONNULL_ARGS__
	void (*destroy)(ctest_reporter_t *);
};
struct ctest_reporter {
	ctest_reporter_ops_t *ops;
};

/**
 * Create a reporter for a test suite.
 *
 * Reporters can then be created for individual tests using the returned test
 * suite reporter.
 *
 * @param reporter  The reporter from which to create the test suite reporter.
 * @param testsuite The test suite for which to create a test suite reporter.
 *
 * @return A new test suite reporter to use to report the progress of tests.
 */
CTEST_ALL_NONNULL_ARGS__
static inline ctest_testsuite_reporter_t *ctest_reporter_report_testsuite(ctest_reporter_t *reporter, ctest_testsuite_t *testsuite)
{
	return (*reporter->ops->report_testsuite)(reporter, testsuite);
}

/**
 * Destroy a reporter, freeing any resources it may have.
 *
 * Any pending test cases will be abandoned and considered to have been
 * cancelled (the objects will still be valid, though). When results to those
 * abandoned tests are reported (via @link ctest_testcase_reporter_complete)
 * the results will be ignored. Any new test cases started (via @link
 * ctest_testcase_reporter_start) will also be ignored.
 *
 * @param reporter The reporter to destroy.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void ctest_reporter_destroy(ctest_reporter_t *reporter)
{
	return (*reporter->ops->destroy)(reporter);
}

#ifdef __cplusplus
}
#endif
#endif /* CTEST__EXEC__REPORTER_H__INCLUDED__ */
