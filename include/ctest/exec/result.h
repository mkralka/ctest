/**
 * The result of running a test case.
 *
 * Then a test case has completed, the result of running the test is reported
 * using a @link ctest_result_t object.
 */
#ifndef CTEST__EXEC__RESULT_H__INCLUDED__
#define CTEST__EXEC__RESULT_H__INCLUDED__

#include <ctest/_annotations.h>
#include <ctest/exec/failure.h>
#include <ctest/exec/output.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The different types of results from completed tests.
 */
typedef enum ctest_result_type ctest_result_type_t;
enum ctest_result_type {
	/**
	 * The test completed successfully.
	 */
	CTEST_RESULT_PASS,

	/**
	 * The test failed, usually due to an explicit assertion.
	 *
	 * Details of the failure can be found in the associated
	 * <code>ctest_failure_t</code> object.
	 */
	CTEST_RESULT_FAIL,

	/**
	 * The test was not executed because a precondition for the test was
	 * not satisfied.
	 *
	 * A skipped test is not considered a failure and does not prevent a
	 * test suite from completed successfully.
	 */
	CTEST_RESULT_SKIPPED,

	/**
	 * The test experienced an unexpected error condition during its run
	 * caused by the unit test framework and not the test itself.
	 *
	 * For example, a unit test may have failed because a system call used
	 * by the runner (e.g., memory allocation) may have failed. */
	CTEST_RESULT_ERROR,
};

/**
 * Details about the result of running a unit test.
 */
typedef struct ctest_result ctest_result_t;
struct ctest_result {
	/**
	 * The type of result (success, failure, etc.).
	 */
	ctest_result_type_t type;

	/**
	 * If the test wrote anything to <code>stdout</code> or
	 * <code>stderr</code>, it will be captured and contained here
	 * (interleaved).
	 */
	const ctest_output_t *output;

	/**
	 * Details of the test failure, if applicable.
	 *
	 * This is only non-<code>NULL</code> if type is one of:
	 * <ul>
	 *   <li><code>CTEST_RESULT_FAIL</code></li>
	 *   <li><code>CTEST_RESULT_SKIP</code></li>
	 *   <li><code>CTEST_RESULT_ERROR</code><li>
	 * <ul>
	 */
	ctest_failure_t *failure;
};

/**
 * Create a new ctest_result_t, initialized as a successful result.
 *
 * The returned result object can be modified using the
 * <code>ctest_result_set_<i>xxx</i></code> methods.
 *
 * @return A new <code>ctest_result_t</code> object, initialized as a success,
 *         or <code>NULL</code> if an error occurs while creating or
 *         initializing the result.
 */
extern ctest_result_t *ctest_result_create_empty(void);

/**
 * Update the type and failure associated with a result.
 *
 * If an existing failure was associated with the result, the resources
 * associated with the failure will be released. If the associated of a failure
 * doesn't make sense with the result type (e.g.,
 * <code>CTEST_RESULT_PASS</code>), then the passed failure must be
 * <code>NULL</code>.
 *
 * @param result  The <code>ctest_result_t</code> to update.
 * @param type    The type of result.
 * @param failure The new failure to associated with the result, or
 *                <code>NULL</code> to remove any association. Ownership of
 *                <code>failure</code> is passed on to <code>result</code>, if
 *                successful.
 *
 * @return Zero if result is updated, non-zero if the update failed.
 */
CTEST_NONNULL_ARGS__(1)
extern int ctest_result_set_failure(ctest_result_t *result, ctest_result_type_t type, ctest_failure_t *failure);

/**
 * Update the output associated with a result.
 *
 * If any existing output was associated with the result, the resources
 * associated with that output will be released.
 *
 * @param result The <code>ctest_result_t</code> to update.
 * @param output The new output to associated with the result, or
 *               <code>NULL</code> to remove any association. Ownership of
 *               <code>output</code> is passed on to <code>result</code>, if
 *               successful.
 *
 * @return Zero if result is updated, non-zero if the update failed.
 */
CTEST_NONNULL_ARGS__(1)
extern int ctest_result_set_output(ctest_result_t *result, ctest_output_t *output);

/**
 * Destroy a <code>ctest_result_t</code> object, freeing resources associated
 * with it.
 *
 * @param result The <code>ctest_result_t</code> object to destroy.
 */
CTEST_ALL_NONNULL_ARGS__
extern void ctest_result_destroy(ctest_result_t *result);

#ifdef __cplusplus
}
#endif

#endif /* CTEST__EXEC__RESULT_H__INCLUDED__ */
