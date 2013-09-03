/**
 * \file
 *
 * \brief Main header
 */

#pragma once
#ifndef __ny__
#define __ny__

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
#define NY_VERSION_MAJOR 0u

/**
 * \brief Minor version number
 */
#define NY_VERSION_MINOR 0u

/**
 * \brief Patch version number
 */
#define NY_VERSION_PATCH 0u

/**
 * \brief Context structure
 */
struct ny {
	struct ev_loop *loop; /**< Event loop */
	struct ny_error error; /**< Last error */
};

/**
 * \brief Get major version number
 *
 * \return Major version number
 */
extern unsigned int ny_version_major() nothrow;

/**
 * \brief Get minor version number
 *
 * \return Minor version number
 */
extern unsigned int ny_version_minor() nothrow;

/**
 * \brief Get patch version number
 *
 * \return Patch version number
 */
extern unsigned int ny_version_patch() nothrow;

/**
 * \brief Initialise context
 *
 * \param[out] ctx Pointer to context structure
 *
 * \return Zero on success or non-zero on error
 */
#define ny_init(ctx) ( \
	(ny_version_major() == NY_VERSION_MAJOR \
		&& ny_version_minor() >= NY_VERSION_MINOR) \
		? ny_init_(ctx) \
		: ny_error_set(&(ctx)->error, NY_ERROR_DOMAIN_NYAN, NY_ERROR_VERSION), -1 \
)

extern int ny_init_(struct ny *restrict ctx) nothrow;

/**
 * \brief Destroy context
 *
 * \param[in,out] ctx Pointer to context structure
 */
extern void ny_destroy(struct ny *restrict ctx) nothrow;

#if defined __cplusplus
}
#endif

#endif