#include <stdlib.h>
#include <string.h>

#include <ctest/_annotations.h>
#include <ctest/exec/stacktrace.h>

#include "serialization.h"
#include "stacktrace.h"
#include "utils.h"

/**
 * Determine the amount of storage required to represent
 * <code>stacktrace</code>.
 *
 * If the number of bytes returned are allocated, the resulting buffer can be
 * passed to <code>stacktrace_storage_format</code> and be populated with a
 * <code>ctest_stacktrace_t</code> object that will be identical to
 * <code>stacktrace</code>.
 *
 * @param stacktrace The stacktrace for which to determine its storage size.
 *
 * @return The amount of storage required to represent <code>stacktrace</code>.
 */
CTEST_ALL_NONNULL_ARGS__
size_t stacktrace_storage_size(const ctest_stacktrace_t *stacktrace)
{
	const size_t frames_length = stacktrace->length;
	size_t ofs = sizeof(stacktrace) + sizeof(stacktrace->frames[0]) * frames_length;
	size_t i;

	for (i = 0; i < frames_length; ++i) {
		const ctest_stackframe_t *const stackframe = stacktrace->frames + i;
		if (stackframe->filename != NULL) {
			ofs = serialize_pad_size(ofs, alignmentof(char));
			ofs += strlen(stackframe->filename) + 1;
		}
	}

	return ofs;
}

/**
 * Format the buffer referenced by <code>buf</code> so that it represents a
 * <code>ctest_stacktrace_t</code> that is identical to <code>stacktrace</code>.
 *
 * if successful, <code>buf</code> can be cast to and used as a
 * <code>ctest_stacktrace_t</code>
 *
 * if unsuccessful, the contents of <code>buf</code> are undefined.
 *
 * @param buf        The buffer in which to format <code>stacktrace</code>.
 * @param len        The length of <code>buf</code>.
 * @param stacktrace The stacktrace to write (format) into <code>buf</code>.
 *
 * @return The number of bytes written to <code>buf</code>, or a negative number
 *         if there was insufficient space in <code>buf</code> to store the
 *         result.
 */
CTEST_ALL_NONNULL_ARGS__
int stacktrace_storage_format(void *buf, size_t len, const ctest_stacktrace_t *stacktrace)
{
	ctest_stacktrace_t *const dst = buf;
	const size_t frames_length = stacktrace->length;
	size_t ofs = sizeof(stacktrace) + sizeof(stacktrace->frames[0]) * frames_length;
	size_t i;

	if (ofs > len)
		return -1;
	dst->length = frames_length;

	for (i = 0; i < frames_length; ++i) {
		const ctest_stackframe_t *const src_stackframe = stacktrace->frames + i;
		ctest_stackframe_t *const dst_stackframe = dst->frames + i;

		if (src_stackframe->filename != NULL) {
			size_t file_ofs = serialize_pad_size(ofs, alignmentof(char));
			size_t file_len = strlen(src_stackframe->filename) + 1;

			ofs = file_ofs + file_len;
			if (ofs > len)
				return -1;
			dst_stackframe->filename = buf + file_ofs;
			memcpy(buf + file_ofs, src_stackframe->filename, file_len);
		} else {
			dst_stackframe->filename = NULL;
		}
		dst_stackframe->addr = src_stackframe->addr;
		dst_stackframe->line = src_stackframe->line;
	}

	return ofs;
}

/**
 * Serialize a buffer, in place, containing a <code>ctest_stacktrace_t</code>.
 *
 * The resultant serialization is the same size as the input and can be copied
 * to another location in memory (or sent over the wire). The changes made
 * during serialization can be undone using
 * <code>stacktrace_storage_deserialize</code>.
 *
 * If unsuccessful, the contents of <code>buf</code> are undefined.
 *
 * @param buf The buffer containing the <code>ctest_stacktrace_t</code> to be
 *            serialized (and will contain the serialized stacktrace on return).
 * @param len The length of <code>buf</code>.
 *
 * @return Zero on success, non-zero on failure.
 */
CTEST_ALL_NONNULL_ARGS__
int stacktrace_storage_serialize(void *buf, size_t unused(len))
{
	ctest_stacktrace_t *const stacktrace = buf;
	const size_t frames_length = stacktrace->length;
	size_t i;

	for (i = 0; i < frames_length; ++i) {
		ctest_stackframe_t *const stackframe = stacktrace->frames + i;

		if (stackframe->filename != NULL)
			stackframe->filename = serialize_rel_ptr_from_abs(stackframe->filename, buf);
	}

	return 0;
}

/**
 * Deserialize a buffer, in place, containing a serialized
 * <code>ctest_stacktrace_t</code>.
 *
 * This undoes the in-place serialization performed by
 * <code>stacktrace_storage_serialize</code>. After this call,
 * <code>buf</code> conforms to all the requirements of a
 * <ode>ctest_stacktrace_t</code> (e.g., contained in a contiguous block).
 *
 * If unsuccessful, the state of <code>buf</code> is undefined.
 *
 * @param buf The buffer containing the serialized
 *            <code>ctest_stacktrace_t</code> to be deserialized (and will
 *            contain the deserialized stacktrace on return).
 * @param len The length of <code>buf</code>.
 *
 * @return Zero on success, non-zero on failure.
 */
CTEST_ALL_NONNULL_ARGS__
int stacktrace_storage_deserialize(void *buf, size_t unused(len))
{
	ctest_stacktrace_t *const stacktrace = buf;
	const size_t frames_length = stacktrace->length;
	size_t i;

	for (i = 0; i < frames_length; ++i) {
		ctest_stackframe_t *const stackframe = stacktrace->frames + i;

		if (stackframe->filename != NULL)
			stackframe->filename = serialize_abs_ptr_from_rel(stackframe->filename, buf);
	}

	return 0;
}

/**
 * Destroy an existing <code>ctest_stacktrace_t</code>, releasing any resources
 * it maintains.
 *
 * @param stacktrace The <code>ctest_stacktrace_t</code> to destroy.
 */
CTEST_ALL_NONNULL_ARGS__
void ctest_stacktrace_destroy(ctest_stacktrace_t *stacktrace)
{
	/* Everything in stacktrace is in the same block of memory. */
	(void)free(stacktrace);
}
