/**
 * \file
 *
 * \brief Memory allocator
 */

#pragma once
#ifndef __ny_alloc__
#define __ny_alloc__

#if defined __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * \brief Allocation pool
 */
struct ny_alloc {
	uint8_t *pool; /**< Object pool */
	size_t memsize; /**< Size of memory allocation */
	uint32_t free; /**< Index of first free object */
	uint16_t size; /**< Object size */
};

/**
 * \brief Initialise allocation pool
 *
 * \param[out] alloc Allocation pool
 * \param[in] ny Ny context
 * \param[in] number Capacity as number of objects
 * \param[in] size Object size
 *
 * \return Zero on success or non-zero on error
 */
extern int ny_alloc_init(struct ny_alloc *restrict alloc,
	struct ny *restrict ny, uint32_t number, uint16_t size);

/**
 * \brief Destroy allocation pool
 *
 * \param[in,out] alloc Allocation pool
 */
extern void ny_alloc_destroy(struct ny_alloc *restrict alloc);

/**
 * \brief Acquire object from pool
 *
 * \param[in,out] alloc Allocation pool
 *
 * \return Pointer to allocated object or null on error
 */
extern void *ny_alloc_acquire(struct ny_alloc *restrict alloc);

/**
 * \brief Release object back into pool
 *
 * \param[in,out] alloc Allocation pool
 * \param[in] object Object
 */
extern void ny_alloc_release(struct ny_alloc *restrict alloc,
	void *restrict object);

#if defined __cplusplus
}
#endif

#endif
