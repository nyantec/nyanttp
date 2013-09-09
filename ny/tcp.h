#pragma once
#ifndef __ny_tcp__
#define __ny_tcp__

#if defined __cplusplus
extern "C" {
#endif

#include <netinet/ip6.h>

#include <ev.h>

#include <defy/bool>

#include <ny/ny.h>
#include <ny/error.h>
#include <ny/alloc.h>

struct ny_tcp;
struct ny_tcp_conn;

/**
 * \brief TCP listener context
 */
struct ny_tcp {
	void *data; /**< User data */
	struct ny *ny; /**< Context structure */
	struct ny_alloc alloc_conn; /**< Connection memory pool */
	struct ev_io io; /**< I/O watcher */
	void (*event_tcp_error)(struct ny_tcp *restrict, struct ny_error const *restrict);
	bool (*event_tcp_connect)(struct ny_tcp *restrict, struct sockaddr_in6 const *restrict); /**< Connect event handler */

	void (*event_conn_error)(struct ny_tcp_conn *restrict, struct ny_error const *restrict);
	void (*event_conn_destroy)(struct ny_tcp_conn *restrict);
	void (*event_conn_readable)(struct ny_tcp_conn *restrict);
	void (*event_conn_writable)(struct ny_tcp_conn *restrict);
	bool (*event_conn_timeout)(struct ny_tcp_conn *restrict);
	int goat; /**< Reserve file descriptor */
};

/**
 * \brief TCP connection context
 */
struct ny_tcp_conn {
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
extern int ny_tcp_init(struct ny_tcp *restrict tcp, struct ny *restrict ny, char const *restrict node, char const *restrict service);

/**
 * \brief Destroy TCP context
 *
 * \param[in,out] tcp Pointer to TCP listener
 */
extern void ny_tcp_destroy(struct ny_tcp *restrict tcp);

extern int ny_tcp_listen(struct ny_tcp *restrict tcp);

extern void ny_tcp_conn_destroy(struct ny_tcp_conn *restrict conn);

extern void ny_tcp_conn_touch(struct ny_tcp_conn *restrict conn);

#if defined __cplusplus
}
#endif

#endif
