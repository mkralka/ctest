#ifndef PRIVATE__DYNAMIC_OPS_H__INCLUDED__
#define PRIVATE__DYNAMIC_OPS_H__INCLUDED__

#include <stdarg.h>

#include <ctest/_annotations.h>

#define CTEST_DYNAMIC_OPS_SYMBOL__ ctest_dynamic_ops

typedef enum ctest_dynamic_ops_abort_type__ ctest_dynamic_ops_abort_type_t;
enum ctest_dynamic_ops_abort_type__ {
	CTEST_DYNAMIC_OPS_ABORT_NONE = 0,
	CTEST_DYNAMIC_OPS_ABORT_FAIL,
	CTEST_DYNAMIC_OPS_ABORT_SKIP,
};

/**
 * Dynamic operations that the testing framework exposes to the tests.
 *
 * This is the way tests interact with the framework. Unlike interfaces like the
 * execution hooks (<code>ctest_exec_hooks_t</code>), this is a stable interface
 * that should not change as new features are added to the testing framework.
 */
typedef struct ctest_dynamic_ops ctest_dynamic_ops_t;
typedef const struct ctest_dynamic_ops_ops ctest_dynamic_ops_ops_t;
struct ctest_dynamic_ops_ops {
	CTEST_VPRINTF__(4)
	void (*report_failure)(ctest_dynamic_ops_t *, const char *, int, const char *, va_list);

	CTEST_NORETURN__
	void (*abort)(ctest_dynamic_ops_t *, ctest_dynamic_ops_abort_type_t);
};
struct ctest_dynamic_ops {
	ctest_dynamic_ops_ops_t *ops;
};

CTEST_VPRINTF__(4)
static inline void ctest_dynamic_ops_report_failure_va(ctest_dynamic_ops_t *dynamic_ops, const char *file, int line, const char *fmt, va_list fmt_params)
{
	(*dynamic_ops->ops->report_failure)(dynamic_ops, file, line, fmt, fmt_params);
}

CTEST_PRINTF__(4, 5)
static inline void ctest_dynamic_ops_report_failure(ctest_dynamic_ops_t *dynamic_ops, const char *file, int line, const char *fmt, ...)
{
	va_list fmt_params;
	va_start(fmt_params, fmt);
	ctest_dynamic_ops_report_failure_va(dynamic_ops, file, line, fmt, fmt_params);
	va_end(fmt_params);
}

CTEST_NORETURN__
static inline void ctest_dynamic_ops_abort(ctest_dynamic_ops_t *dynamic_ops, ctest_dynamic_ops_abort_type_t abort_type)
{
	(*dynamic_ops->ops->abort)(dynamic_ops, abort_type);
}

#endif /* PRIVATE__DYNAMIC_OPS_H__INCLUDED__ */
