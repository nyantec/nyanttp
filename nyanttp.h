/**
 * \file
 *
 * \brief Main header
 */

#pragma once
#ifndef __nyanttp__
#define __nyanttp__

#if defined __cplusplus
extern "C" {
#endif

#include <ev.h>

#include <defy/nothrow>
#include <defy/restrict>

#include "error.h"

/**
 * \brief Major version number
 */
#define NYANTTP_VERSION_MAJOR 0u

/**
 * \brief Minor version number
 */
#define NYANTTP_VERSION_MINOR 0u

/**
 * \brief Context structure
 */
struct nyanttp {
	struct ev_loop *loop; /**< Event loop */
	struct nyanttp_error error; /**< Last error */
};

/**
 * \brief Get major version number
 *
 * \return Major version number
 */
extern unsigned int nyanttp_version_major() nothrow;

/**
 * \brief Get minor version number
 *
 * \return Minor version number
 */
extern unsigned int nyanttp_version_minor() nothrow;

/**
 * \brief Initialise context
 *
 * \param ctx Pointer to context structure
 *
 * \return Zero on success or non-zero on error
 */
#define nyanttp_init(ctx) ( \
	(nyanttp_version_major() == NYANTTP_VERSION_MAJOR \
		&& nyanttp_version_minor() >= NYANTTP_VERSION_MINOR) \
		? nyanttp_init_(ctx) \
		: ((ctx)->error.domain = NYANTTP_ERROR_DOMAIN_NYAN, \
			(ctx)->error.code = NYANTTP_ERROR_VERSION, -1) \
	)

extern int nyanttp_init_(struct nyanttp *restrict ctx) nothrow;

/**
 * \brief Destroy context
 *
 * \param ctx Pointer to context structure
 */
extern void nyanttp_destroy(struct nyanttp *restrict ctx) nothrow;

#if defined __cplusplus
}
#endif

#endif
