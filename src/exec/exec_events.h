#ifndef PRIVATE__EXEC_EVENTS_H__INCLUDED__
#define PRIVATE__EXEC_EVENTS_H__INCLUDED__

#include <stdint.h>

#include <ctest/_annotations.h>
#include <ctest/exec/failure.h>
#include <ctest/exec/stage.h>

#include "poll_handler.h"

/*
 * Execution Event Consumer
 */

/**
 * An execution event consumer is is a sink for execution events.
 *
 * Execution events are comprised by components of execution hooks, but do not
 * completely contain all the details of the execution hooks. With some
 * additional out-of-band information, execution events can be used as a shim
 * to transfer execution hooks.
 */
typedef struct exec_event_consumer exec_event_consumer_t;
typedef const struct exec_event_consumer_ops exec_event_consumer_ops_t;
struct exec_event_consumer_ops {
	CTEST_ALL_NONNULL_ARGS__
	void (*on_stage_change)(exec_event_consumer_t *, ctest_stage_t);

	CTEST_ALL_NONNULL_ARGS__
	void (*on_failure)(exec_event_consumer_t *, ctest_failure_t *);
};
struct exec_event_consumer {
	exec_event_consumer_ops_t *ops;
};

/**
 * Notify an <code>exec_event_consumer_t</code> of a stage change event.
 *
 * @param consumer The consumer to notify.
 * @param stage    The stage in the change event, indicating the new stage of
 *                 execution.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void exec_event_consumer_on_stage_change(exec_event_consumer_t *consumer, ctest_stage_t stage)
{
	return (*consumer->ops->on_stage_change)(consumer, stage);
}

/**
 * Notify an <code>exec_event_consumer_t</code> of received failure.
 *
 * @param consumer The consumer to notify.
 * @param stage    The failure from the event, describing a failure that has
 *                 occurred. Ownership of the failure is passed on to the
 *                 consumer.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void exec_event_consumer_on_failure(exec_event_consumer_t *consumer, ctest_failure_t *failure)
{
	return (*consumer->ops->on_failure)(consumer, failure);
}

/*
 * Execution Event Writer
 */

/**
 * An <code>exec_event_writer_t</code> is a sink (consumer) of execution events
 * that serializes the events and writes them to file descriptor.
 *
 * The serialized events can be read using an <code>exec_event_reader_t</code>.
 */
typedef struct exec_event_writer exec_event_writer_t;
struct exec_event_writer {
	exec_event_consumer_t consumer_base;
	int fd;
};

/**
 * Write a stage change event.
 *
 * This is a blocking call that will return when the failure event is completely
 * written to the writer's file descriptor.
 *
 * @param writer The writer to which to write the stage change event.
 * @param stage  The stage to include in the written stage change event.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void exec_event_writer_on_stage_change(exec_event_writer_t *writer, ctest_stage_t stage)
{
	return exec_event_consumer_on_stage_change(&writer->consumer_base, stage);
}

/**
 * Write a failure event.
 *
 * This is a blocking call that will return when the failure event is completely
 * written to the writer's file descriptor.
 *
 * @param writer  The writer to which to write the failure event.
 * @param failure The failure to include in the written failure event.
 */
CTEST_ALL_NONNULL_ARGS__
static inline void exec_event_writer_on_failure(exec_event_writer_t *writer, ctest_failure_t *failure)
{
	return exec_event_consumer_on_failure(&writer->consumer_base, failure);
}

CTEST_ALL_NONNULL_ARGS__
extern void exec_event_writer_init(exec_event_writer_t *writer, int fd);

CTEST_ALL_NONNULL_ARGS__
extern void exec_event_writer_destroy(exec_event_writer_t *writer);

/*
 * Execution Event Reader
 */

/**
 * The structure of the header that denotes an event.
 */
typedef struct exec_event_msg_header__ exec_event_msg_header_t__;
struct exec_event_msg_header__ {
	uint16_t type;
	uint16_t length;
};


/**
 * An <code>exec_event_reader_t</code> is a source (producer) of execution
 * events that reads serialized events from a file descriptor, deserializes
 * them, and passes them along to a <code>exec_event_consumer_t</code>.
 *
 * An <code>exec_event_reader_t</code> is a <code>event_t</code> and is
 * designed to work within a event polling framework (e.g., <code>poll</code>,
 * <code>epoll</code>, or <code>select</code>).
 */
typedef struct exec_event_reader exec_event_reader_t;
struct exec_event_reader {
	poll_handler_t poll_handler_base;
	exec_event_consumer_t *consumer;
	int fd;

	void *buf;              /* Location to store read data -- NULL to drop */
	size_t ofs;             /* Number of bytes read into buf */
	size_t cap;             /* Capacity of buf */
	size_t len;             /* Number of bytes in the message. */

	/* Per-state storage. */
	union {
		struct {
			exec_event_msg_header_t__ header;
		} read_header;
		struct {
			union {
				ctest_stage_t stage;
				ctest_failure_t *failure;
			} msg;
			void (*on_done)(exec_event_reader_t *);
		} read_body;
	} state;
	void (*on_done)(exec_event_reader_t *);
};

/**
 * Notify a reader that data is available to be read from its file descriptor.
 *
 * @param reader The reader for which data is available.
 *
 * @return The number of bytes consumed from the file descriptor, or a negative
 *         number if reading from the file descriptor failed.
 */
CTEST_ALL_NONNULL_ARGS__
static inline int exec_event_reader_on_data_available(exec_event_reader_t *reader)
{
	return poll_handler_on_data_available(&reader->poll_handler_base);
}

CTEST_ALL_NONNULL_ARGS__
void exec_event_reader_init(exec_event_reader_t *reader, int fd, exec_event_consumer_t *consumer);

CTEST_ALL_NONNULL_ARGS__
void exec_event_reader_destroy(exec_event_reader_t *reader);

#endif /* PRIVATE__EXEC_EVENTS_H__INCLUDED__ */
