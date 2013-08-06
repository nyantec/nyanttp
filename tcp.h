#pragma once
#ifndef __nyanttp_tcp__
#define __nyanttp_tcp__

#if defined __cplusplus
extern "C" {
#endif

#include <netinet/ip6.h>

#include <ev.h>

#include <defy/bool>

#include "error.h"
#include "nyanttp.h"

struct nyanttp_tcp;
struct nyanttp_tcp_conn;

/**
 * \brief TCP listener context
 */
struct nyanttp_tcp {
	void *data; /**< User data */
	struct ev_io io; /**< I/O listener */
	bool (*event_connect)(struct nyanttp_tcp *restrict, struct sockaddr_in6 *restrict); /**< Connect event handler */
	void (*event_readable)(struct nyanttp_tcp_conn *restrict);
	void (*event_writable)(struct nyanttp_tcp_conn *restrict);
};

/**
 * \brief TCP connection context
 */
struct nyanttp_tcp_conn {
	void *data; /**< User data */
	struct nyanttp_tcp *tcp; /**< TCP listener */
	struct ev_io io; /**< I/O watcher */
	/* TODO: Event listeners */
};



/**
 * \brief Initialise TCP context
 *
 * \param tcp Pointer to TCP listener
 * \param node Host name or address
 * \param service Service name or port number
 */
extern int nyanttp_tcp_init(struct nyanttp_tcp *restrict tcp, char const *restrict node, char const *restrict service);

/**
 * \brief Destroy TCP context
 *
 * \param tcp Pointer to TCP listener
 */
extern void nyanttp_tcp_destroy(struct nyanttp_tcp *restrict tcp);

extern int nyanttp_tcp_listen(struct nyanttp_tcp *restrict tcp, struct nyanttp *restrict ctx);

#if defined __cplusplus
}
#endif

#endif