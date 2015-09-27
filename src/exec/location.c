#include <string.h>

#include "location.h"
#include "utils.h"
#include "serialization.h"

/**
 * Determine the amount of storage require to represent <code>location</code>.
 *
 * If the number of bytes returned are allocated, the resulting buffer can be
 * passed to <code>location_storage_format</code> and be populated with a
 * <code>ctest_location_t</code> object that will be identical to
 * <code>location</code>.
 *
 * @param location The location for which to determine its storage size.
 *
 * @return The amount of storage required to represent <code>location</code>.
 */
CTEST_ALL_NONNULL_ARGS__
size_t location_storage_size(const ctest_location_t *location)
{
	size_t size;

	size = sizeof(ctest_location_t);

	if (location->filename != NULL) {
		size = serialize_pad_size(size, alignmentof(char)) + strlen(location->filename) + 1;
	}

	return size;
}

/**
 * Format the buffer referenced by <code>buf</code> so that it represent a
 * <code>ctest_location_t</code> that is identical to <code>location</code>.
 *
 * If successful, <code>buf</code> can be cast to and used as a
 * <code>ctest_location_t</code>.
 *
 * If unsuccessful, the contents of <code>buf</code> are undefined.
 *
 * @param buf      The buffer in which to format <code>location</code>.
 * @param len      The length of <code>buf</code>.
 * @param location The location to write (format) into <code>buf</code>.
 *
 * @return The number of bytes written to <code>buf</code>, or a negative
 *         number if there is insufficient space in <code>buf</code> to store
 *         the result.
 */
CTEST_ALL_NONNULL_ARGS__
int location_storage_format(void *buf, size_t len, const ctest_location_t *location)
{
	ctest_location_t *const dst = buf;
	size_t ofs = 0;

	ofs += sizeof(ctest_location_t);
	if (ofs > len)
		return -1;

	if (location->filename != NULL) {
		size_t filename_ofs = serialize_pad_size(ofs, alignmentof(char));
		size_t filename_len = strlen(location->filename) + 1;
		ofs = filename_ofs + filename_len;

		if (ofs > len)
			return -1;

		dst->filename = buf + filename_ofs;
		memcpy(buf + filename_ofs, location->filename, filename_len);
	} else {
		dst->filename = NULL;
	}

	dst->line = location->line;
	return ofs;
}

/**
 * Serialize a buffer in place, containing a <code>ctest_location_t</code>.
 *
 * The resultant serialization is the same size as the input and can be copied
 * to another location in memory (or sent over the wire). The changes made
 * during serialization can be undone using
 * <code>location_storage_deserialize</code>.
 *
 * If unsuccessful, the contents of <code>buf</code> are undefined.
 *
 * @param buf The buffer containing the <code>ctest_location_t</code> to be
 *            serialized (and will contain the serialized failure on return).
 * @param len The length of <code>buf</code>.
 *
 * @return Zero on success, non-zero on failure.
 */
CTEST_ALL_NONNULL_ARGS__
int location_storage_serialize(void *buf, size_t unused(len))
{
	ctest_location_t *location = buf;

	if (location->filename != NULL)
		location->filename = serialize_rel_ptr_from_abs(location->filename, buf);

	return 0;
}

/**
 * Deserialize a buffer, in place, containing a serialized
 * <code>ctest_location_t</code>.
 *
 * This undoes the in-place serialization performed by
 * <code>location_storage_serialize</code>. After this call,
 * <code>buf</code> conforms to all the requirements of a
 * <code>ctest_location_t</code> (e.g., contained in a contiguous block).
 *
 * If unsuccessful, the contents of <code>buf</code> are undefined.
 *
 * @param buf The buffer containing the serialized <code>ctest_location_t</code>
 *            to be deserialized (and will contain the deserialized failure
 *            on return).
 * @param len The length of <code>buf</code>.
 *
 * @return Zero on success, non-zero on failure.
 */
CTEST_ALL_NONNULL_ARGS__
int location_storage_deserialize(void *buf, size_t unused(len))
{
	ctest_location_t *location = buf;

	if (location->filename != NULL)
		location->filename = serialize_abs_ptr_from_rel(location->filename, buf);

	return 0;
}
