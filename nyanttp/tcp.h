#pragma once
#ifndef __ny_tcp__
#define __ny_tcp__

#if defined __cplusplus
extern "C" {
#endif

#include <netinet/ip6.h>

#include <ev.h>

#include <defy/bool>

#include <nyanttp/ny.h>
#include <nyanttp/error.h>
#include <nyanttp/alloc.h>

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
 */
extern int ny_tcp_init(struct ny_tcp *restrict tcp, struct ny *restrict ny,
	char const *restrict node, char const *restrict service);

/**
 * \brief Destroy TCP context
 *
 * \param[in,out] tcp Pointer to TCP listener
 */
extern void ny_tcp_destroy(struct ny_tcp *restrict tcp);

extern int ny_tcp_listen(struct ny_tcp *restrict tcp);

extern void ny_tcp_con_destroy(struct ny_tcp_con *restrict con);

extern void ny_tcp_con_touch(struct ny_tcp_con *restrict con);

extern ssize_t ny_tcp_con_recv(struct ny_tcp_con *restrict con,
	void *restrict buffer, size_t length);

extern ssize_t ny_tcp_con_send(struct ny_tcp_con *restrict con,
	void const *restrict buffer, size_t length);

extern ssize_t ny_tcp_cond_sendfile(struct ny_tcp_con *restrict con,
	int fd, off_t offset, size_t length);

#if defined __cplusplus
}
#endif

#endif
