/**
 * \file
 *
 * \brief Percent‐encode URL
 */

#pragma once
#ifndef __ny_urlencode__
#define __ny_urlencode__

#if defined __cplusplus
extern "C" {
#endif

#include <sys/types.h>

extern ssize_t ny_urlencode(char *restrict out, char const *restrict in,
	size_t outlen, size_t inlen);

#if defined __cplusplus
}
#endif

#endif
