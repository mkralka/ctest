#ifndef CTEST__EXEC__OUTPUT_H__INCLUDED__
#define CTEST__EXEC__OUTPUT_H__INCLUDED__

#include <stddef.h>

#include <ctest/_annotations.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The output captured during a test.
 *
 * This typically represents the data written to stdout and stderr, interleaved.
 */
typedef struct ctest_output ctest_output_t;
struct ctest_output {
	/** The length of the data field, in bytes.  */
	size_t length;

	/* The data representing the output.
	 *
	 * Since output may be binary, this is not implicitly NUL terminated.
	 * */
	char data[];
};

/**
 * Create a new ctest_output_t object with a given capacity.
 *
 * @param capacity The capacity of the returned object.
 *
 * @return A new ctest_output_t object with the specified capacity, or NULL if
 *         one could not be created.
 */
extern ctest_output_t *ctest_output_create(size_t capacity);


/**
 * Resize the capacity of a ctest_output_t object.
 *
 * The capacity may be increased or decreased. If decreased, the output is
 * truncated at the new size. If increased, the new storage space is initialized
 * to NUL bytes.
 *
 * NOTE: If p_output refers to a NULL pointer, this behaves as though the result
 * of ctest_output_create(capacity) was stored in *p_output.
 *
 * @param p_output A pointer to a ctest_output_t reference whose capacity is to
 *                 be updated. If successful, this may be updated to refer to
 *                 a different ctest_output_t object. On a failure, this is
 *                 unchanged.
 * @param capacity The new capacity of the ctest_output_t object referenced by
 *                 p_output.
 * @return Zero on success, non-zero on failure.
 */
CTEST_ALL_NONNULL_ARGS__
extern int ctest_output_resize(ctest_output_t **p_output, size_t capacity);

/**
 * Destroy a ctest_output_t object, freeing resources associated with it.
 *
 * @param output The ctest_output_t object to destroy.
 */
CTEST_ALL_NONNULL_ARGS__
extern void ctest_output_destroy(ctest_output_t *output);

#ifdef __cplusplus
}
#endif

#endif /* CTEST__EXEC__OUTPUT_H__INCLUDED__ */
