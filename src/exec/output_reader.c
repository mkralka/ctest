#include <string.h>
#include <unistd.h>

#include "output_reader.h"
#include "utils.h"

static inline output_reader_t *upcast_poll_handler__(poll_handler_t *poll_handler)
{
	return containerof(poll_handler, output_reader_t, poll_handler_base);
}

static int op_on_data_available__(poll_handler_t *poll_handler)
{
	output_reader_t *const reader = upcast_poll_handler__(poll_handler);
	const int fd = reader->fd;
	ctest_output_t *output = reader->output;
	size_t length = reader->length;
	int rc;

	if (output == NULL) {
		reader->output = output = ctest_output_create(128);
	} else if (length >= output->length) {
		ctest_output_resize(&output, reader->length * 2);
		reader->output = output;
	}

	if (output == NULL || length >= output->length) {
		/* Unable to allocate storage for output, drain. */
		/* TODO: Keep most recent output? */
		char buf[4096];
		rc = read(fd, buf, sizeof(buf));
	} else {
		if ((rc = read(fd, output->data+length, output->length-length)) >= 0)
			reader->length += rc;
	}

	return rc;
}

static void op_on_close__(poll_handler_t *unused(poll_handler))
{
}

/**
 * Initialize a new <code>output_reader_t</code>.
 *
 * The <code>output_reader_t</code> should be destroyed, when it is no longer
 * needed, using <code>output_reader_destroy</code>.
 *
 * @param reader The <code>output_reader_t</code> to initialize.
 * @param fd     The file descriptor from which to read output. Ownership of the
 *               file descriptor is transferred to the reader and will be closed
 *               when the reader is destroyed.
 */
CTEST_ALL_NONNULL_ARGS__
void output_reader_init(output_reader_t *reader, int fd)
{
	static poll_handler_ops_t ops = {
		&op_on_data_available__,
		&op_on_close__,
	};

	memset(reader, 0, sizeof(*reader));
	reader->poll_handler_base.ops = &ops;
	reader->fd = fd;
	reader->output = NULL;
}

/**
 * Destroy an existing <code>output_reader_t</code>, previously initialized with
 * <code>output_reader_init</code>.
 *
 * After destroying a <code>output_reader_t</code>, it should not be used until
 * re-initialized (by <code>output_reader_t</code>).
 *
 * @param reader The <code>output_reader_t</code> to destroy.
 */
CTEST_ALL_NONNULL_ARGS__
void output_reader_destroy(output_reader_t *reader)
{
	(void)close(reader->fd);
	if (reader->output != NULL) {
		ctest_output_destroy(reader->output);
	}
	memset(reader, 0, sizeof(*reader));
}

/**
 * Build a <code>ctest_output_t</code> object from the data read (so far) from
 * the file descriptor.
 *
 * After building a <code>ctest_output_t</code>, the reader state is reset and
 * any new data read from the file descriptor will be collected in a new
 * <code>ctest_output_t</code> object.
 *
 * @param reader The <code>output_reader_t</code> object from which to build an
 *               <code>ctest_output_t<code> object.
 *
 * @return A <code>ctest_output_t</code> object, representing the data read from
 *         the reader's file descriptor, or NULL if no data has been read.
 */
CTEST_ALL_NONNULL_ARGS__
ctest_output_t *output_reader_build(output_reader_t *reader)
{
	const size_t length = reader->length;
	ctest_output_t *output = reader->output;

	if (output == NULL || length == 0)
		return NULL;

	reader->output = NULL;
	reader->length = 0;

	ctest_output_resize(&output, length+1);
	output->data[length] = '\0';
	return output;
}
