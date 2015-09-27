#ifndef PRIVATE__OUTPUT_READER_H__INCLUDED__
#define PRIVATE__OUTPUT_READER_H__INCLUDED__

#include <ctest/exec/output.h>

#include "poll_handler.h"

/**
 * A <code>ctest_output_t</code> builder that consumes data from a file
 * descriptor.
 *
 * <code>output_reader_t</code> implements the <code>poll_handler_t</code>
 * interface so it can be notified when data becomes available.
 */
typedef struct output_reader output_reader_t;
struct output_reader {
	poll_handler_t poll_handler_base;
	int fd;
	ctest_output_t *output;
	size_t length;
};

/**
 * Notify an output reader that data is available to be read from its file
 * descriptor.
 *
 * @param reader The output reader fro which data is available.
 *
 * @return The number of bytes consumed from the file descriptor, or a negative
 *         number if reading from the file descriptor failed.
 */
CTEST_ALL_NONNULL_ARGS__
static inline int output_reader_on_data_available(output_reader_t *reader)
{
	return poll_handler_on_data_available(&reader->poll_handler_base);
}

CTEST_ALL_NONNULL_ARGS__
extern void output_reader_init(output_reader_t *reader, int fd);

CTEST_ALL_NONNULL_ARGS__
extern void output_reader_destroy(output_reader_t *reader);

CTEST_ALL_NONNULL_ARGS__
extern ctest_output_t *output_reader_build(output_reader_t *reader);

#endif /* PRIVATE__OUTPUT_READER_H__INCLUDED__ */
