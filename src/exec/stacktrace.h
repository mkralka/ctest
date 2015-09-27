#ifndef PRIVATE__STACKTRACE_H__INCLUDED__
#define PRIVATE__STACKTRACE_H__INCLUDED__

#include <stddef.h>

#include <ctest/_annotations.h>
#include <ctest/exec/stacktrace.h>

CTEST_ALL_NONNULL_ARGS__
extern size_t stacktrace_storage_size(const ctest_stacktrace_t *stacktrace);

CTEST_ALL_NONNULL_ARGS__
extern int stacktrace_storage_format(void *buf, size_t len, const ctest_stacktrace_t *stacktrace);

CTEST_ALL_NONNULL_ARGS__
extern int stacktrace_storage_serialize(void *buf, size_t len);

CTEST_ALL_NONNULL_ARGS__
extern int stacktrace_storage_deserialize(void *buf, size_t len);

#endif /* PRIVATE__STACKTRACE_H__INCLUDED__ */

