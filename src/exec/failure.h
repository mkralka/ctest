#ifndef PRIVATE__FAILURE_H__INCLUDED__
#define PRIVATE__FAILURE_H__INCLUDED__

#include <stddef.h>

#include <ctest/_annotations.h>
#include <ctest/exec/failure.h>

CTEST_ALL_NONNULL_ARGS__
extern size_t failure_storage_size(const ctest_failure_t *failure);

CTEST_ALL_NONNULL_ARGS__
extern int failure_storage_format(void *buf, size_t len, const ctest_failure_t *failure);

CTEST_ALL_NONNULL_ARGS__
extern int failure_storage_serialize(void *buf, size_t len);

CTEST_ALL_NONNULL_ARGS__
extern int failure_storage_deserialize(void *buf, size_t len);

#endif /* PRIVATE__FAILURE_H__INCLUDED__ */
