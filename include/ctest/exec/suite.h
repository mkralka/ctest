/**
 * The Test Suite
 *
 * A <em>test suite</em> is a collection of related <em>tests</em> that
 * typically, but not always, work together to validate the same translation
 * unit (or module).
 *
 * A <em>test</em> is the sequence of operational and verification steps to be
 * perform on a given input; it is associated with <strong>exactly one</strong>
 * test suite. A test typically, but not always, validates one aspect of the
 * code being test (e.g., that an protocol parser properly handles decoding
 * multi-byte integers correctly). A test is associated with one or more test
 * cases.
 *
 * A <em>test case</em> is the lowest level, most elemental unit of a test suite
 * representing a single sequence of operational and verification steps
 * performed on a single, specific, input; it is test case is associated
 * with <strong>exactly one</strong> test.
 *
 * It may be helpful to think of a test as a <em>function</em> and a test case
 * as the <em>invocation of that function</em> (with parameters). A suite might
 * then be thought of as a translation unit, containing function definitions.
 */
#ifndef CTEST__EXEC__SUITE_H__INCLUDED__
#define CTEST__EXEC__SUITE_H__INCLUDED__

#include <stddef.h>

#include <ctest/_annotations.h>
#include <ctest/exec/exec_hooks.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The smallest unit of testing.
 *
 * A test case is a single sequence of operational and verification steps
 * performed on a single input. It is always run in a single thread, from start
 * to finish and, depending on the test, it may be executed in parallel
 * (although independently) of other test cases.
 *
 * Test cases are always associated with a single test.
 */
typedef struct ctest_testcase ctest_testcase_t;

/**
 * A sequence of actions to test or validate code.
 *
 * A test is the sequence of operational and verification steps to perform on
 * a given input. One or more test cases comprise a test, allowing the same test
 * to be be performed on multiple inputs.
 *
 * Test are always associated with a single test suite.
 */
typedef struct ctest_test ctest_test_t;

/**
 * A collection of related tests.
 *
 * A test suite is a collection of related tests, typically validating the same
 * module.
 */
typedef struct ctest_testsuite ctest_testsuite_t;

/*
 * Test Case
 */
typedef const struct ctest_testcase_ops ctest_testcase_ops_t;
struct ctest_testcase_ops {
	CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
        const char *(*get_name)(ctest_testcase_t *);

	CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
	ctest_test_t *(*get_test)(ctest_testcase_t *);

	CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
        void (*execute)(ctest_testcase_t *, ctest_exec_hooks_t *);

};
struct ctest_testcase {
        ctest_testcase_ops_t *ops;
};

/**
 * Get the human-readable name of the test case.
 *
 * <em>Ownership of the returend value is not transferred to the caller; the
 * returned value should not be freed.<em>
 *
 * @param testcase The test case for which to get the human-readable name.
 * @return The human-readable name of the test case.
 */
CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static inline const char *ctest_testcase_get_name(ctest_testcase_t *testcase)
{
        return (*testcase->ops->get_name)(testcase);
}

/**
 * Get the test with which the test case is associated.
 *
 * @param testcase The test case for which to get the associated test.
 * @return The test with which the test case is associated.
 */
CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static inline ctest_test_t *ctest_testcase_get_test(ctest_testcase_t *testcase)
{
        return (*testcase->ops->get_test)(testcase);
}

/**
 * Execute the test case.
 *
 * @param testcase The test case to execute.
 * @param hooks    The failure hooks to handle test failures.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void ctest_testcase_execute(ctest_testcase_t *testcase, ctest_exec_hooks_t *hooks)
{
        return (*testcase->ops->execute)(testcase, hooks);
}


/*
 * Test
 */
typedef const struct ctest_test_ops ctest_test_ops_t;
struct ctest_test_ops {
	CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
	const char *(*get_name)(ctest_test_t *);

	CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
	ctest_testsuite_t *(*get_testsuite)(ctest_test_t *);

	CTEST_ALL_NONNULL_ARGS__
	size_t (*get_testcase_count)(ctest_test_t *);

	CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
	ctest_testcase_t *const*(*get_testcases)(ctest_test_t *);
};
struct ctest_test {
	ctest_test_ops_t *ops;
};

/**
 * Get the human-readable name of the test.
 *
 * @param test The test for which to get the human-readable name.
 * @return The human-readable name of the test.
 */
CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static inline const char *ctest_test_get_name(ctest_test_t *test)
{
	return (*test->ops->get_name)(test);
}

/**
 * Get the test suite with which the test is associated.
 *
 * @param test The test for which to get the associated test suite.
 * @return The test suite with which the test is associated.
 */
CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static inline ctest_testsuite_t *ctest_test_get_testsuite(ctest_test_t *test)
{
	return (*test->ops->get_testsuite)(test);
}

/**
 * Get the number of test cases associated with the test.
 *
 * @param test The test for which to get the number of test cases.
 * @return The number of test cases associated with the test.
 */
CTEST_ALL_NONNULL_ARGS__
static inline size_t ctest_test_get_testcase_count(ctest_test_t *test)
{
	return (*test->ops->get_testcase_count)(test);
}

/**
 * Get the test cases associated with the test.
 *
 * Ownership of the test cases remains within the associated suite; references
 * should not be kept after the suite has been destroyed.
 *
 * @param test The test for which to get the associated test cases.
 * @return The test cases associated with the test.
 */
CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static inline ctest_testcase_t *const*ctest_test_get_testcases(ctest_test_t *test)
{
	return (*test->ops->get_testcases)(test);
}

/*
 * Test Suite
 */
typedef const struct ctest_testsuite_ops ctest_testsuite_ops_t;
struct ctest_testsuite_ops {
	CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
	const char *(*get_name)(ctest_testsuite_t *);

	CTEST_ALL_NONNULL_ARGS__
	size_t (*get_test_count)(ctest_testsuite_t *);

	CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
	ctest_test_t *const*(*get_tests)(ctest_testsuite_t *);

	CTEST_ALL_NONNULL_ARGS__
	void (*destroy)(ctest_testsuite_t *);
};
struct ctest_testsuite {
	ctest_testsuite_ops_t *ops;
};

/**
 * Get the human-readable name of the test suite.
 *
 * @param testsuite The test suite for which to get the human-readable name.
 * @return The human-readable name of the test suite.
 */
CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static inline const char *ctest_testsuite_get_name(ctest_testsuite_t *testsuite)
{
	return (*testsuite->ops->get_name)(testsuite);
}

/**
 * Get the number of tests associated with the test suite.
 *
 * @param testsuite The test suite for which to get the number of associated
 *                  tests.
 * @return The number of tests associated with the test suite.
 */
CTEST_ALL_NONNULL_ARGS__
static inline size_t ctest_testsuite_get_test_count(ctest_testsuite_t *testsuite)
{
	return (*testsuite->ops->get_test_count)(testsuite);
}

/**
 * Get the tests associated with the test suite.
 *
 * Ownership of the tests remains within the suite; references should not be
 * kept after the suite has been destroyed.
 *
 * @param testsuite The test suite for which to get the associated tests.
 * @return The tests associated with the test suite.
 */
CTEST_ALL_NONNULL_ARGS__ CTEST_RETURNS_NONNULL__
static inline ctest_test_t *const*ctest_testsuite_get_tests(ctest_testsuite_t *testsuite)
{
	return (*testsuite->ops->get_tests)(testsuite);
}

/**
 * Destroy a test suite, freeing any resources it may have.
 *
 * @param testsuite The test suite to destroy.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void ctest_testsuite_destroy(ctest_testsuite_t *testsuite)
{
	return (*testsuite->ops->destroy)(testsuite);
}

#ifdef __cplusplus
}
#endif
#endif /* CTEST__EXEC__SUITE_H__INCLUDED__ */
