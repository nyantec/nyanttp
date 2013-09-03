/**
 * \file
 *
 * \brief Memory allocator
 */

#pragma once
#ifndef __nyanttp_alloc__
#define __nyanttp_alloc__

#if defined __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <defy/restrict>

/**
 * \brief Allocation pool
 */
struct nyanttp_alloc {
	void *pool; /**< Memory pool */
	uint32_t free; /**< Index of first free object */
	uint16_t size; /**< Object size */
};

/**
 * \brief Initialise allocation pool
 *
 * \param[out] alloc Allocation pool
 * \param[in] number Capacity as number of objects
 * \param[in] size Object size
 *
 * \return Zero on success or non-zero on error
 */
extern int nyanttp_alloc_init(struct nyanttp_alloc *restrict alloc, uint32_t number, uint16_t size);

/**
 * \brief Destroy allocation pool
 *
 * \param[in,out] alloc Allocation pool
 */
extern void nyanttp_alloc_destroy(struct nyanttp_alloc *restrict alloc);

/**
 * \brief Acquire object from pool
 *
 * \param[in,out] alloc Allocation pool
 *
 * \return Pointer to allocated object or null on error
 */
extern void *nyanttp_alloc_acquire(struct nyanttp_alloc *restrict alloc);

/**
 * \brief Release object back into pool
 *
 * \param[in,out] alloc Allocation pool
 * \param[in] object Object
 */
extern void nyanttp_alloc_release(struct nyanttp_alloc *restrict alloc, void *restrict object);

#if defined __cplusplus
}
#endif

#endif
