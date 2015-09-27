#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctest/exec/failure.h>

#include "utils.h"
#include "failure.h"
#include "location.h"
#include "serialization.h"
#include "stacktrace.h"

static size_t description_length__(const char *fmt, va_list fmt_params)
{
	va_list fmt_params_copy;
	size_t result;

	va_copy(fmt_params_copy, fmt_params);
	result = vsnprintf(NULL, 0, fmt, fmt_params_copy);
	va_end(fmt_params_copy);

	return result;
}

static size_t storage_size__(ctest_stage_t unused(stage), size_t description_len, const ctest_location_t *location, const ctest_stacktrace_t *stacktrace)
{
	size_t size = sizeof(ctest_failure_t);

	if (description_len > 0)
		size = serialize_pad_size(size, alignmentof(char)) + description_len;

	if (location != NULL)
		size = serialize_pad_size(size, alignmentof(ctest_location_t)) + location_storage_size(location);

	if (stacktrace != NULL)
		size = serialize_pad_size(size, alignmentof(ctest_stacktrace_t)) + stacktrace_storage_size(stacktrace);

	return size;
}

static int storage_format__(void *buf, size_t len, ctest_stage_t stage, size_t description_len, const ctest_location_t *location, const ctest_stacktrace_t *stacktrace)
{
	ctest_failure_t *const failure = buf;
	size_t ofs = sizeof(ctest_failure_t);
	int rc;

	if (ofs > len)
		return -1;

	/* Because certain fields are aligned, some bytes may be effectively
	 * random. Rather than figuring out which ones, just zero out
	 * everything. */
	memset(buf, '\0', len);

	failure->stage = stage;

	if (description_len > 0) {
		const size_t description_ofs = serialize_pad_size(ofs, alignmentof(char));
		ofs = description_ofs + description_len;
		if (ofs > len)
			return -1;
		failure->description = buf + description_ofs;
	} else {
		failure->description = NULL;
	}

	if (location != NULL) {
		ofs = serialize_pad_size(ofs, alignmentof(ctest_location_t));

		if (ofs > len)
			return -1;

		failure->location = buf + ofs;
		if ((rc = location_storage_format(buf + ofs, len - ofs, location)) < 0)
			return rc;
		ofs += rc;
	} else {
		failure->location = NULL;
	}

	if (stacktrace != NULL) {
		ofs = serialize_pad_size(ofs, alignmentof(ctest_stacktrace_t));

		if (ofs > len)
			return -1;

		failure->stacktrace = buf + ofs;
		if ((rc = stacktrace_storage_format(buf + ofs, len - ofs, stacktrace)) < 0)
			return rc;
		ofs += rc;
	} else {
		failure->stacktrace = NULL;
	}

	return ofs;
}

/**
 * Determine the amount of storage required to represent <code>failure</code>.
 *
 * If the number of bytes returned are allocated, the resulting buffer can be
 * passed to <code>failure_storage_format</code> and be populated with a
 * <code>ctest_failure_t</code> object that will be identical to
 * <code>failure</code>.
 *
 * @param failure The failure for which to determine its storage size.
 *
 * @return The amount of storage required to represent <code>failure</code>.
 */
CTEST_ALL_NONNULL_ARGS__
size_t failure_storage_size(const ctest_failure_t *failure)
{
	size_t description_len = failure->description != NULL ? strlen(failure->description) + 1 : 0;
	return storage_size__(failure->stage, description_len, failure->location, failure->stacktrace);
}

/**
 * Format the buffer referenced by <code>buf</code> so that it represents a
 * <code>ctest_failure_t</code> that is identical to <code>failure</code>.
 *
 * if successful, <code>buf</code> can be cast to and used as a
 * <code>ctest_failure_t</code>
 *
 * if unsuccessful, the contents of <code>buf</code> are undefined.
 *
 * @param buf     The buffer in which to format <code>failure</code>.
 * @param len     The length of <code>buf</code>.
 * @param failure The failure to write (format) into <code>buf</code>.
 *
 * @return The number of bytes written to <code>buf</code>, or a negative number
 *         if there was insufficient space in <code>buf</code> to store the
 *         result.
 */
CTEST_ALL_NONNULL_ARGS__
int failure_storage_format(void *buf, size_t len, const ctest_failure_t *failure)
{
	size_t description_len = failure->description != NULL ? strlen(failure->description) + 1 : 0;
	int rc;

	if ((rc = storage_format__(buf, len, failure->stage, description_len, failure->location, failure->stacktrace)) >= 0) {
		if (failure->description != NULL)
			memcpy((char *)((ctest_failure_t *)buf)->description, failure->description, description_len);
	}

	return rc;
}

/**
 * Serialize a buffer, in place, containing a <code>ctest_failure_t</code>.
 *
 * The resultant serialization is the same size as the input and can be copied
 * to another location in memory (or sent over the wire). The changes made
 * during serialization can be undone using
 * <code>failure_storage_deserialize</code>.
 *
 * If unsuccessful, the contents of <code>buf</code> are undefined.
 *
 * @param buf The buffer containing the <code>ctest_failure_t</code> to be
 *            serialized (and will contain the serialized failure on return).
 * @param len The length of <code>buf</code>.
 *
 * @return Zero on success, non-zero on failure.
 */
int failure_storage_serialize(void *buf, size_t len)
{
	ctest_failure_t *const failure = buf;
	if (failure->description != NULL) {
		failure->description = serialize_rel_ptr_from_abs(failure->description, buf);
	}

	if (failure->location != NULL) {
		void *const location = (void *)failure->location;
		if (location_storage_serialize(location, len - (location - buf)) != 0)
			return -1;
		failure->location = serialize_rel_ptr_from_abs(failure->location, buf);
	}

	if (failure->stacktrace != NULL) {
		void *const stacktrace = (void *)failure->stacktrace;
		if (stacktrace_storage_serialize(stacktrace, len - (stacktrace - buf)) != 0)
			return -1;
		failure->stacktrace = serialize_rel_ptr_from_abs(failure->stacktrace, buf);
	}
	return 0;
}

/**
 * Deserialize a buffer, in place, containing a serialized
 * <code>ctest_failure_t</code>.
 *
 * This undoes the in-place serialization performed by
 * <code>failure_storage_serialize</code>. After this call,
 * <code>buf</code> conforms to all the requirements of a
 * <ode>ctest_failure_t</code> (e.g., contained in a contiguous block).
 *
 * If unsuccessful, the state of <code>buf</code> is undefined.
 *
 * @param buf The buffer containing the serialized <code>ctest_failure_t</code>
 *            to be deserialized (and will contain the deserialized failure on
 *            return).
 * @param len The length of <code>buf</code>.
 *
 * @return Zero on success, non-zero on failure.
 */
int failure_storage_deserialize(void *buf, size_t len)
{
	ctest_failure_t *const failure = buf;

	if (failure->description != NULL) {
		failure->description = serialize_abs_ptr_from_rel(failure->description, buf);
	}

	if (failure->location != NULL) {
		void *const location = serialize_abs_ptr_from_rel(failure->location, buf);
		failure->location = location;
		if (location_storage_deserialize(location, len - (location - buf)) != 0)
			return -1;
	}

	if (failure->stacktrace != NULL) {
		void *const stacktrace = serialize_abs_ptr_from_rel(failure->stacktrace, buf);
		failure->stacktrace = stacktrace;
		if (stacktrace_storage_deserialize(stacktrace, len = (stacktrace - buf)) != 0)
			return -1;
	}

	return 0;
}

/**
 * Create a new <code>ctest_failure_t</code> with all fields specified.
 *
 * The created <code>ctest_failure_t</code> is allocated on the heap and all
 * memory associated with it can be released using <code>free</code>.
 *
 * @param stage           The stage in which the failure occurred.
 * @param description_fmt The printf-style format specifier from which to build
 *                        the description.
 * @param location        The (source code) location where the failure occurred.
 * @param stacktrace      The stacktrace to assign to the failure.
 * @param ...             The parameters to <code>description_fmt</code>.
 *
 * @return A new <code>ctest_failure_t</code> with the specified values filled
 *         in (stage, description, location, and stacktrace), or
 *         <code>NULL</code> on error.
 */
CTEST_PRINTF__(2, 5)
ctest_failure_t *ctest_failure_create(ctest_stage_t stage, const char *description_fmt, const ctest_location_t *location, const ctest_stacktrace_t *stacktrace, ...)
{
	va_list description_fmt_params;
	ctest_failure_t *result;

	va_start(description_fmt_params, stacktrace);
	result = ctest_failure_create_va(stage, description_fmt, description_fmt_params, location, stacktrace);
	va_end(description_fmt_params);

	return result;
}

/**
 * Create a new <code>ctest_failure_t</code> with all fields specified.
 *
 * The created <code>ctest_failure_t</code> is allocated on the heap and all
 * memory associated with it can be released using <code>free</code>.
 *
 * @param stage              The stage in which the failure occurred.
 * @param description_fmt    The printf-style format specifier from which to
 *                           build the description.
 * @param description_params The parameters to <code>description_fmt</code>.
 * @param location           The (source code) location where the failure
 *                           occurred.
 * @param stacktrace         The stacktrace to assign to the failure.
 *
 * @return A new <code>ctest_failure_t</code> with the specified values filled
 *         in (stage, description, location, and stacktrace), or
 *         <code>NULL</code> on error.
 */
CTEST_VPRINTF__(2)
ctest_failure_t *ctest_failure_create_va(ctest_stage_t stage, const char *description_fmt, va_list description_fmt_params, const ctest_location_t *location, const ctest_stacktrace_t *stacktrace)
{
	ctest_failure_t *result;
	size_t description_len = description_length__(description_fmt, description_fmt_params) + 1;
	size_t size = storage_size__(stage, description_len, location, stacktrace);
	void *buf;
	int rc;

	if ((buf = calloc(1, size)) == NULL)
		return NULL;
	result = buf;

	if ((rc = storage_format__(buf, size, stage, description_len, location, stacktrace)) < 0)
		goto format_failed;

	vsnprintf((char *)result->description, description_len, description_fmt, description_fmt_params);
	return result;

format_failed:
	(void)free(buf);
	return NULL;
}

/**
 * Destroy an existing <code>ctest_failure_t</code>, releasing any resources it
 * maintains.
 *
 * @param failure The <code>ctest_failure_t</code> to destroy.
 */
void ctest_failure_destroy(ctest_failure_t *failure)
{
	(void)free(failure);
}

#if 0
/**
 * Clone an existing failure object.
 *
 * Create an exact copy of an existing failure object. Since this ensures the
 * returned object is allocated in contiguous block of memory (that can be
 * released by calling <code>free()</code> on the returned object), it is
 * useful in building new failure objects. Simply populate a temporary
 * <code>ctest_failure_t</code> object with appropriate values, then
 * use <code>ctest_failure_clone</code> to build the final object.
 */
CTEST_ALL_NONNULL_ARGS__
ctest_failure_t *ctest_failure_clone(const ctest_failure_t *failure)
{
}
#endif
