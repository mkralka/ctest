#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctest/exec/result.h>

ctest_result_t *ctest_result_create_empty(void)
{
	ctest_result_t *result;

	if ((result = calloc(1, sizeof(*result))) != NULL) {
		result->type = CTEST_RESULT_PASS;
		result->output = NULL;
		result->failure = NULL;
	}

	return result;
}

CTEST_NONNULL_ARGS__(1)
int ctest_result_set_failure(ctest_result_t *result, ctest_result_type_t type, ctest_failure_t *failure)
{
	/* FIXME: Reject changes with mismatch of type and failure presence.
	 *        (e.g., a PASS with a failure). */
	result->type = type;
	if (result->failure != NULL)
		ctest_failure_destroy(result->failure);
	result->failure = failure;

	return 0;
}

CTEST_NONNULL_ARGS__(1)
extern int ctest_result_set_output(ctest_result_t *result, ctest_output_t *output)
{
	if (result->output != NULL)
		ctest_output_destroy((ctest_output_t *)result->output);
	result->output = output;
	return 0;
}

CTEST_ALL_NONNULL_ARGS__
void ctest_result_destroy(ctest_result_t *result)
{
	if (result->output != NULL)
		ctest_output_destroy((ctest_output_t *)result->output);

	if (result->failure != NULL)
		ctest_failure_destroy(result->failure);

	memset(result, 0, sizeof(*result));
	(void)free(result);
}
