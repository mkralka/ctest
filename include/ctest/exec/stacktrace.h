#ifndef CTEST__EXEC__STACKTRACE_H__INCLUDED__
#define CTEST__EXEC__STACKTRACE_H__INCLUDED__

#include <stddef.h>

#include <ctest/_annotations.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * An stack frame in the call stack describing exactly where the error
 * occurred during a test failure.
 */
typedef struct ctest_stackframe ctest_stackframe_t;
struct ctest_stackframe {
	const void *addr;
	const char *filename;
	int line;
};

/**
 * The call stack, describing where an error occurred during a test failure.
 */
typedef struct ctest_stacktrace ctest_stacktrace_t;
struct ctest_stacktrace {
	/**
	 * The number of stack frames.
	 */
	size_t length;

	/**
	 * The stack frames, from deepest point to shallowest.
	 */
	ctest_stackframe_t frames[];
};

CTEST_ALL_NONNULL_ARGS__
extern void ctest_stacktrace_destroy(ctest_stacktrace_t *stacktrace);

#ifdef __cplusplus
}
#endif

#endif /* CTEST__EXEC__STACKTRACE_H__INCLUDED__ */
