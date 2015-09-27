#ifndef PRIVATE__POLL_HANDLER_H__INCLUDED__
#define PRIVATE__POLL_HANDLER_H__INCLUDED__

/**
 * A poll handler responds to data available poll events.
 *
 * When polling a file descriptor (e.g., via <code>poll()</code>,
 * <code>epoll()</code>, or <code>select</code>), handling of the available data
 * can be performed behind the interface of a poll handler. A polling framework
 * will listen for file descriptors and, when data is available, notify the
 * poll handler to read from the associated file descriptor.
 */
typedef struct poll_handler poll_handler_t;
typedef const struct poll_handler_ops poll_handler_ops_t;
struct poll_handler_ops {
	int (*on_data_available)(poll_handler_t *);
	void (*on_close)(poll_handler_t *);
};
struct poll_handler {
	poll_handler_ops_t *ops;
};

/**
 * Notify a poll handler that data is available for reading.
 *
 * @param handler The poll handler to notify.
 *
 * @return The number of bytes consumed, or a negative number on error.
 */
static inline int poll_handler_on_data_available(poll_handler_t *handler)
{
	return (*handler->ops->on_data_available)(handler);
}

/**
 * Notify a poll handler that the data stream has been closed (and no more data
 * will become available).
 *
 * @param handler The poll handler to notify.
 */
static inline void poll_handler_on_close(poll_handler_t *handler)
{
	return (*handler->ops->on_close)(handler);
}

#endif /* PRIVATE__POLL_HANDLER_H__INCLUDED__ */
