#ifndef CTEST__EXEC__FAILURE_H__INCLUDED__
#define CTEST__EXEC__FAILURE_H__INCLUDED__

#include <stdarg.h>

#include <ctest/_annotations.h>
#include <ctest/exec/location.h>
#include <ctest/exec/stage.h>
#include <ctest/exec/stacktrace.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Details about a failure that occurred while running a test.
 *
 * A failure is always stored in one contiguous block of memory and can be
 * freed using <code>free()</code>.
 */
typedef struct ctest_failure ctest_failure_t;
struct ctest_failure {
	/**
	 * The execution stage in which the failure occurred.
	 */
	ctest_stage_t stage;

	/**
	 * A human-readable string describing the failure.
	 */
	const char *description;

	/**
	 * The (source code) location where the failure occurred.
	 */
	ctest_location_t *location;

	/**
	 * The stack trace, from deepest point to shallowest.
	 *
	 * The stack trace is set only if it is available and the test failed.
	 */
	const ctest_stacktrace_t *stacktrace;
};

CTEST_PRINTF__(2, 5)
extern ctest_failure_t *ctest_failure_create(ctest_stage_t stage, const char *description_fmt, const ctest_location_t *location, const ctest_stacktrace_t *stacktrace, ...);

CTEST_VPRINTF__(2)
extern ctest_failure_t *ctest_failure_create_va(ctest_stage_t stage, const char *description_fmt, va_list description_params, const ctest_location_t *location, const ctest_stacktrace_t *stacktrace);

CTEST_ALL_NONNULL_ARGS__
extern void ctest_failure_destroy(ctest_failure_t *failure);

CTEST_ALL_NONNULL_ARGS__
extern ctest_failure_t *ctest_failure_clone(const ctest_failure_t *failure);

#ifdef __cplusplus
}
#endif

#endif /* CTEST__EXEC__FAILURE_H__INCLUDED__ */
