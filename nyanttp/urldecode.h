/**
 * \file
 *
 * \brief Decode percent‐encoded URL
 */

#pragma once
#ifndef __ny_urldecode__
#define __ny_urldecode__

#if defined __cplusplus
extern "C" {
#endif

#include <sys/types.h>

extern ssize_t ny_urldecode(char *restrict out, char const *restrict in,
	size_t outlen, size_t inlen);

#if defined __cplusplus
}
#endif

#endif
