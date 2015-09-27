#include <ctest/tests/assert.h>

#include "dynamic_ops.h"

ctest_dynamic_ops_t *CTEST_DYNAMIC_OPS_SYMBOL__;

CTEST_PRINTF__(3, 4) CTEST_NORETURN__
extern void ctest_fail(const char *file, int line, const char *fmt, ...)
{
	va_list fmt_params;
	va_start(fmt_params, fmt);
	ctest_dynamic_ops_report_failure_va(CTEST_DYNAMIC_OPS_SYMBOL__, file, line, fmt, fmt_params);
	ctest_dynamic_ops_abort(CTEST_DYNAMIC_OPS_SYMBOL__, CTEST_DYNAMIC_OPS_ABORT_FAIL);
}

CTEST_PRINTF__(3, 4) CTEST_NORETURN__
extern void ctest_skip(const char *file, int line, const char *fmt, ...)
{
	va_list fmt_params;
	va_start(fmt_params, fmt);
	ctest_dynamic_ops_report_failure_va(CTEST_DYNAMIC_OPS_SYMBOL__, file, line, fmt, fmt_params);
	ctest_dynamic_ops_abort(CTEST_DYNAMIC_OPS_SYMBOL__, CTEST_DYNAMIC_OPS_ABORT_SKIP);
}
