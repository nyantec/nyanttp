#pragma once
#ifndef __ny_tcp__
#define __ny_tcp__

#if defined __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include <sys/socket.h>
#include <netinet/in.h>

#include <ev.h>

#include <nyanttp/ny.h>
#include <nyanttp/error.h>
#include <nyanttp/alloc.h>

#define NY_TCP_READABLE EV_READ
#define NY_TCP_WRITABLE EV_WRITE

struct ny_tcp;
struct ny_tcp_con;

/**
 * \brief TCP listener context
 */
struct ny_tcp {
	void *data; /**< User data */
	struct ny *ny; /**< Context structure */
	struct ny_alloc alloc_con; /**< Connection memory pool */
	struct ev_io io; /**< I/O watcher */
	void (*tcp_error)(struct ny_tcp *restrict,
		struct ny_error const *restrict);
	bool (*tcp_connect)(struct ny_tcp *restrict,
		struct sockaddr_in6 const *restrict); /**< Connect event handler */

	void (*con_error)(struct ny_tcp_con *restrict,
		struct ny_error const *restrict);
	void (*con_destroy)(struct ny_tcp_con *restrict);
	void (*con_readable)(struct ny_tcp_con *restrict);
	void (*con_writable)(struct ny_tcp_con *restrict);
	bool (*con_timeout)(struct ny_tcp_con *restrict);
	int goat; /**< Reserve file descriptor */
};

/**
 * \brief TCP connection context
 */
struct ny_tcp_con {
	void *data; /**< User data */
	struct ny_tcp *tcp; /**< TCP listener */
	struct ev_timer timer; /**< Timeout watcher */
	struct ev_io io; /**< I/O watcher */
};



/**
 * \brief Initialise TCP context
 *
 * \param[out] tcp Pointer to TCP listener
 * \param[in,out] ny Pointer to context structure
 * \param[in] node Host name or address
 * \param[in] service Service name or port number
 * \param[in] maxcon Maximum number of connections per worker
 */
extern int ny_tcp_init(struct ny_tcp *restrict tcp, struct ny *restrict ny,
	char const *restrict node, char const *restrict service,
	uint_least32_t maxcon);

/**
 * \brief Destroy TCP context
 *
 * \param[in,out] tcp Pointer to TCP listener
 */
extern void ny_tcp_destroy(struct ny_tcp *restrict tcp);

extern int ny_tcp_listen(struct ny_tcp *restrict tcp);

extern void ny_tcp_con_destroy(struct ny_tcp_con *restrict con);

extern void ny_tcp_con_touch(struct ny_tcp_con *restrict con);

extern void ny_tcp_con_events(struct ny_tcp_con *restrict con, 
	int events);

extern ssize_t ny_tcp_con_recv(struct ny_tcp_con *restrict con,
	void *restrict buffer, size_t length);

extern ssize_t ny_tcp_con_send(struct ny_tcp_con *restrict con,
	void const *restrict buffer, size_t length);

extern ssize_t ny_tcp_con_sendfile(struct ny_tcp_con *restrict con,
	int fd, size_t length, off_t offset);

#if defined __cplusplus
}
#endif

#endif
