#ifndef PRIVATE__LOCATION_H__INCLUDED__
#define PRIVATE__LOCATION_H__INCLUDED__

#include <stddef.h>

#include <ctest/_annotations.h>
#include <ctest/exec/location.h>

CTEST_ALL_NONNULL_ARGS__
extern size_t location_storage_size(const ctest_location_t *location);

CTEST_ALL_NONNULL_ARGS__
extern int location_storage_format(void *buf, size_t len, const ctest_location_t *location);

CTEST_ALL_NONNULL_ARGS__
extern int location_storage_serialize(void *buf, size_t len);

CTEST_ALL_NONNULL_ARGS__
extern int location_storage_deserialize(void *buf, size_t len);

#endif /* PRIVATE__LOCATION_H__INCLUDED__ */
