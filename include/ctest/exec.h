#ifndef CTEST__EXEC_H__INCLUDED__
#define CTEST__EXEC_H__INCLUDED__

#include <ctest/_annotations.h>
#include <ctest/exec/exec_hooks.h>
#include <ctest/exec/failure.h>
#include <ctest/exec/output.h>
#include <ctest/exec/reporter.h>
#include <ctest/exec/result.h>
#include <ctest/exec/runner.h>
#include <ctest/exec/stacktrace.h>
#include <ctest/exec/stage.h>
#include <ctest/exec/suite.h>

#ifdef __cplusplus
extern "C" {
#endif

extern ctest_reporter_t *ctest_create_console_reporter(void);
extern ctest_runner_t *ctest_create_direct_runner(void);
extern ctest_runner_t *ctest_create_forking_runner(void);

CTEST_ALL_NONNULL_ARGS__
extern ctest_testsuite_t *ctest_create_testing_testsuite(const char *name);

CTEST_ALL_NONNULL_ARGS__
extern ctest_testsuite_t *ctest_load_testsuite(const char *filename);

#ifdef __cplusplus
}
#endif
#endif /* CTEST__EXEC_H__INCLUDED__ */
