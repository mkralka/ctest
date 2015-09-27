#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <ctest/exec/output.h>

static inline size_t storage_size__(size_t capacity)
{
	return sizeof(ctest_output_t) + capacity * sizeof(((ctest_output_t *)NULL)->data[0]);
}

ctest_output_t *ctest_output_create(size_t capacity)
{
	ctest_output_t *result;

	if ((result = calloc(1, storage_size__(capacity))) != NULL)
		result->length = capacity;

	return result;
}

CTEST_ALL_NONNULL_ARGS__
int ctest_output_resize(ctest_output_t **p_output, size_t capacity)
{
	ctest_output_t *output = *p_output;
	size_t length = output == NULL ? 0 : output->length;

	if ((output = realloc(output, storage_size__(capacity))) == NULL)
		return -1;

	if (length < capacity)
		memset(output->data + length, 0, capacity - length);

	*p_output = output;
	output->length = capacity;
	return 0;
}

CTEST_ALL_NONNULL_ARGS__
void ctest_output_destroy(ctest_output_t *output)
{
	(void)free(output);
}

