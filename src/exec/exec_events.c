#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "exec_events.h"
#include "failure.h"
#include "utils.h"

/**
 * The types for individual events that can be serialized/deserialized.
 */
typedef enum exec_event_type__ exec_event_type_t__;
enum exec_event_type__ {
	EXEC_EVENT_STAGE_CHANGE__,
	EXEC_EVENT_FAILURE__,
};

/*
 * Writer
 */

static inline exec_event_writer_t *upcast_exec_event_writer__(exec_event_consumer_t *consumer)
{
	return containerof(consumer, exec_event_writer_t, consumer_base);
}

static void exec_event_writer_op_on_stage_change__(exec_event_consumer_t *consumer, ctest_stage_t stage)
{
	exec_event_writer_t *const writer = upcast_exec_event_writer__(consumer);
	exec_event_msg_header_t__ header;

	memset(&header, 0, sizeof(header));
	header.length = sizeof(stage);
	header.type = EXEC_EVENT_STAGE_CHANGE__;

	if (write(writer->fd, &header, sizeof(header)) != (int)sizeof(header))
		return;

	if (write(writer->fd, &stage, sizeof(stage)) != (int)sizeof(stage))
		return;
}

static void exec_event_writer_op_on_failure__(exec_event_consumer_t *consumer, ctest_failure_t *failure)
{
	exec_event_writer_t *const writer = upcast_exec_event_writer__(consumer);
	exec_event_msg_header_t__ header;
	size_t len = failure_storage_size(failure);
	char buf[len];
	int rc;

	if ((rc = failure_storage_format(buf, len, failure)) != (int)len) {
		return;
	}
	if ((rc = failure_storage_serialize(buf, len)) != 0) {
		return;
	}

	memset(&header, 0, sizeof(header));
	header.length = len;
	header.type = EXEC_EVENT_FAILURE__;

	if ((rc = write(writer->fd, &header, sizeof(header))) != (int)sizeof(header))
		return;

	if ((rc = write(writer->fd, buf, len)) != (int)len)
		return;
}

/**
 * Initialize a new <code>exec_event_writer_t</code>.
 *
 * The <code>exec_event_writer_t</code> should be destroyed when it is no longer
 * needed using <code>exec_event_writer_destroy</code>
 *
 * @param writer The <code>exec_event_writer_t</code> to initialize.
 * @param fd     The file descriptor to which to write events. Ownership of the
 *               file descriptor is transferred to the writer and will be closed
 *               when the writer is destroyed.
 */
CTEST_ALL_NONNULL_ARGS__
void exec_event_writer_init(exec_event_writer_t *writer, int fd)
{
	static exec_event_consumer_ops_t ops = {
		&exec_event_writer_op_on_stage_change__,
		&exec_event_writer_op_on_failure__,
	};

	memset(writer, 0, sizeof(*writer));
	writer->consumer_base.ops = &ops;
	writer->fd = fd;
}

/**
 * Destroy an existing <code>exec_event_writer_t</code>, previously initialized
 * with <code>exec_event_writer_init</code>.
 *
 * After destroying a <code>exec_event_writer_t</code>, it should not be used
 * until re-initialized (by <code>exec_event_writer_init</code>).
 *
 * @param writer The <code>exec_event_writer_t</code> to initialize.
 */
CTEST_ALL_NONNULL_ARGS__
void exec_event_writer_destroy(exec_event_writer_t *writer)
{
	(void)close(writer->fd);
	writer->fd = -1;
}

/*
 * Reader
 */

static inline exec_event_reader_t *upcast_poll_handler__(poll_handler_t *consumer)
{
	return containerof(consumer, exec_event_reader_t, poll_handler_base);
}

static void reader_on_ignored_msg_done__(exec_event_reader_t *unused(reader))
{
}

static void reader_on_state_change_done__(exec_event_reader_t *reader)
{
	exec_event_consumer_on_stage_change(reader->consumer, reader->state.read_body.msg.stage);
}

static void reader_on_failure_done__(exec_event_reader_t *reader)
{
	if (failure_storage_deserialize(reader->buf, reader->ofs) == 0) {
		exec_event_consumer_on_failure(reader->consumer, reader->buf);
	} else {
		(void)free(reader->buf);
	}
}

static void reader_on_msg_header_done__(exec_event_reader_t *reader);
static void reader_on_msg_body_done__(exec_event_reader_t *reader);

static void reader_prep_next_msg__(exec_event_reader_t *reader)
{
	reader->buf = &reader->state.read_header.header;
	reader->ofs = 0;
	reader->cap = reader->len = sizeof(reader->state.read_header.header);
	reader->on_done = &reader_on_msg_header_done__;
}

static void reader_on_msg_header_done__(exec_event_reader_t *reader)
{
	const exec_event_msg_header_t__ header = reader->state.read_header.header;
	reader->buf = NULL;
	reader->ofs = 0;
	reader->cap = reader->len = header.length;
	reader->state.read_body.on_done = &reader_on_ignored_msg_done__;

	switch(header.type) {
	case EXEC_EVENT_STAGE_CHANGE__:
		if (reader->len >= sizeof(reader->state.read_body.msg.stage)) {
			reader->buf = &reader->state.read_body.msg.stage;
			reader->cap = sizeof(reader->state.read_body.msg.stage);
			reader->state.read_body.on_done = &reader_on_state_change_done__;
		}
		break;

	case EXEC_EVENT_FAILURE__:
		reader->buf = reader->state.read_body.msg.failure = malloc(reader->len);
		reader->state.read_body.on_done = &reader_on_failure_done__;
		break;
	}

	reader->on_done = &reader_on_msg_body_done__;
}

static void reader_on_msg_body_done__(exec_event_reader_t *reader)
{
	(*reader->state.read_body.on_done)(reader);

	if (reader->cap < reader->len) {
		/* More data was provided than expected; drop it it */
		reader->buf = NULL;
		reader->cap = reader->len;
		reader->on_done = &reader_prep_next_msg__;
	} else {
		reader_prep_next_msg__(reader);
	}
}

static int reader_op_on_data_available__(poll_handler_t *handler)
{
	exec_event_reader_t *const reader = upcast_poll_handler__(handler);
	void *buf = reader->buf + reader->ofs;
	size_t len = reader->cap - reader->ofs;
	char garbage[1024];
	int rc;

	if (buf == NULL) {
		/* Bytes being ignored */
		buf = garbage;
		if (len > sizeof(garbage))
			len = sizeof(garbage);
	}

	if ((rc = read(reader->fd, buf, len)) < 0)
		return rc;

	reader->ofs += rc;

	if (reader->ofs >= reader->cap) {
		(*reader->on_done)(reader);
	}
	return rc;
}

static void reader_op_on_close__(poll_handler_t *unused(handler))
{
}

 /**
 * Initialize a new <code>exec_event_reader_t</code>.
 *
 * The <code>exec_event_reader_t</code> should be destroyed, when it is no
 * longer needed, using <code>exec_event_reader_destroy</code>.
 *
 * @param reader The <code>exec_event_reader_t</code> to initialize.
 * @param fd     The file descriptor from which to read events. Ownership of the
 *               file descriptor is transferred to the reader and will be closed
 *               when the reader is destroyed.
 */
CTEST_ALL_NONNULL_ARGS__
void exec_event_reader_init(exec_event_reader_t *reader, int fd, exec_event_consumer_t *consumer)
{
	static poll_handler_ops_t ops = {
		&reader_op_on_data_available__,
		&reader_op_on_close__,
	};

	memset(reader, 0, sizeof(*reader));
	reader->poll_handler_base.ops = &ops;
	reader->fd = fd;
	reader->consumer = consumer;
	reader_prep_next_msg__(reader);
}

/**
 * Destroy an existing <code>exec_event_reader_t</code>, previously initialized
 * with <code>exec_event_reader_init</code>.
 *
 * After destroying a <code>exec_event_reader_t</code>, it should not be used
 * until re-initialized (by <code>exec_event_reader_init</code>).
 *
 * @param reader The <code>exec_event_reader_t</code> to destroy.
 */
CTEST_ALL_NONNULL_ARGS__
void exec_event_reader_destroy(exec_event_reader_t *reader)
{
	void *const lower_bound = reader;
	void *const upper_bound = reader + 1;

	(void)close(reader->fd);

	if (reader->buf != NULL && reader->buf < lower_bound && reader->buf >= upper_bound) {
		/* reader->buf doesn't refer to a memory location within the
		 * reader; it must have been allocated. Free it */
		(void)free(reader->buf);
	}
	memset(reader, 0, sizeof(*reader));
	reader->fd = -1;
}
