/**
 * \file
 *
 * \brief Utility functions
 */

#pragma once
#ifndef __ny_util__
#define __ny_util__

#if defined __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include <defy/perfect>

/**
 * \brief Align value
 *
 * \param[in] value Value to align
 * \param[in] align Alignment (must be a power of two)
 *
 * \return Aligned value
 */
inline perfect size_t ny_util_align(size_t size, size_t align) {
	return (size + align - (size_t) 1) & ~(align - (size_t) 1);
}

#if defined __cplusplus
}
#endif

#endif
