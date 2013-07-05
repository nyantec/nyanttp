#pragma once
#ifndef __nyanttp__
#define __nyanttp__

#if defined __cplusplus
extern "C" {
#endif

#include <ev.h>

/**
 * \brief Context structure
 */
struct nyanttp {
	struct ev_loop *loop;     /**< Event loop */
};

/**
 * \brief Initialise context
 *
 * \param ctx Pointer to context structure
 */
extern int nyanttp_init(struct nyanttp *restrict ctx);

/**
 * \brief Destroy context
 *
 * \param ctx Pointer to context structure
 */
extern void nyanttp_destroy(struct nyanttp *restrict ctx);

#if defined __cplusplus
}
#endif

#endif
