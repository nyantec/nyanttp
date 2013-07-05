#pragma once
#ifndef __nyanttp_tcp__
#define __nyanttp_tcp__

#if defined __cplusplus
extern "C" {
#endif

#include <ev.h>

#include "nyanttp.h"

/**
 * \brief TCP listener context
 */
struct nyanttp_tcp {
	struct ev_io io; /**< I/O listener */
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
