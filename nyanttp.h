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
 */
extern unsigned int nyanttp_version_major() nothrow;

/**
 * \brief Get minor version number
 */
extern unsigned int nyanttp_version_minor() nothrow;

/**
 * \brief Initialise context
 *
 * \param ctx Pointer to context structure
 */
extern int nyanttp_init(struct nyanttp *restrict ctx) nothrow;

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
