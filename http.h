#pragma once
#ifndef __nyanttp_http__
#define __nyanttp_http__

#if defined __cplusplus
extern "C" {
#endif

#include <ev.h>

#include "nyanttp.h"

/**
 * \brief HTTP listener context
 */
struct nyanttp_http {
	struct ev_io io; /**< I/O listener */
};

/**
 * \brief Initialise HTTP context
 *
 * \param http Pointer to HTTP listener
 * \param node Host name or address
 * \param service Service name or port number
 */
extern int nyanttp_http_init(struct nyanttp_http *restrict http, char const *restrict node, char const *restrict service);

/**
 * \brief Destroy HTTP context
 *
 * \param http Pointer to HTTP listener
 */
extern void nyanttp_http_destroy(struct nyanttp_http *restrict http);

extern int nyanttp_http_listen(struct nyanttp_http *restrict http, struct nyanttp *restrict ctx);

#if defined __cplusplus
}
#endif

#endif
