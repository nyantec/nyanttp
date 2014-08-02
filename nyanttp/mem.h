/**
 * \file
 *
 * \brief Memory helper functions
 */

#pragma once
#ifndef __ny_mem__
#define __ny_mem__

#if defined __cplusplus
extern "C" {
#endif

#include <stddef.h>

extern void *ny_mem_alloc(size_t length);
extern int ny_mem_free(void *restrict memory, size_t length);

#if defined __cplusplus
}
#endif

#endif
