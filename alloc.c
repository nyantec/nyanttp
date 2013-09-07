/**
 * \file
 *
 * \internal
 */

#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>

#include <defy/const>
#include <defy/expect>
#include <defy/nil>
#include <defy/pure>

#include "ny.h"
#include "alloc.h"

static defy_const size_t align(size_t size, size_t alignment) {
	return (size + alignment - (size_t) 1) & ~(alignment - (size_t) 1);
}

static defy_const size_t max(size_t a, size_t b) {
	return a > b ? a : b;
}

static defy_pure void *index(struct ny_alloc const *restrict alloc, uint32_t idx) {
	/* Calculate octet offset */
	size_t offset = alloc->size * idx;

	/* Upper bound */
	assert(offset < alloc->alloc);

	return alloc->pool + offset;
}

static defy_pure uint32_t pointer(struct ny_alloc const *restrict alloc, void *ptr) {
	/* Lower bound */
	assert(((uintptr_t) ptr) >= ((uintptr_t) alloc->pool));

	/* Calculate octet offset */
	size_t offset = ((uintptr_t) ptr) - ((uintptr_t) alloc->pool);

	/* Alignment */
	assert(offset % alloc->size == 0);

	/* Upper bound */
	assert(offset + alloc->size < alloc->alloc);

	return offset / alloc->size;
}

int ny_alloc_init(struct ny_alloc *restrict alloc, struct ny *restrict ny,
	uint32_t number, uint16_t size) {
	assert(alloc);
	assert(ny);
	assert(number);
	assert(size);

	int _;

	int status = -1;

	if (unlikely(number > UINT32_C(4194304) || size > UINT16_C(1024))) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_ERRNO, ERANGE);
		goto exit;
	}

	/* Minimum size of 4 bytes, aligned to word size */
	alloc->size = align(max(size, sizeof (uint32_t)), sizeof (void *));

	/* Size of payload without guard pages */
	size_t payload = align(number * alloc->size, ny->page_size);

	/* Size of memory allocation */
	alloc->alloc = payload + 2 * ny->page_size;

	/* Adjust object number to fill up last page */
	size_t actual = payload / alloc->size;

	/* Allocate memory for pool */
	alloc->raw = mmap(nil, alloc->alloc, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (unlikely(alloc->raw == MAP_FAILED)) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
		goto exit;
	}

	/* Setup lower guard page */
	if (unlikely(mprotect(alloc->raw, ny->page_size, PROT_NONE))) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
		goto unmap;
	}

	/* Setup upper guard page */
	if (unlikely(mprotect(alloc->raw + (alloc->alloc - ny->page_size),
		ny->page_size, PROT_NONE))) {
		ny_error_set(&ny->error, NY_ERROR_DOMAIN_ERRNO, errno);
		goto unmap;
	}

	alloc->pool = alloc->raw + ny->page_size;

	/* Advise the OS about the access pattern */
	posix_madvise(alloc->pool, payload,
		POSIX_MADV_SEQUENTIAL | POSIX_MADV_WILLNEED);

	/* Initialise free list */
	alloc->free = 0;
	for (uint_least32_t iter = 0; iter < actual - 1; ++iter)
		*(uint32_t *) index(alloc, iter) = iter + 1;

	/* Sentinel */
	*(uint32_t *) index(alloc, actual - 1) = UINT32_MAX;

	status = 0;
	goto exit;

unmap:
	/* Unmap memory pool */
	_ = munmap(alloc->pool, alloc->alloc);
	assert(!_);

exit:
	return status;
}

void ny_alloc_destroy(struct ny_alloc *restrict alloc) {
	assert(alloc);
	assert(alloc->pool);
	assert(alloc->alloc);

	int _;

	/* Unmap memory pool */
	_ = munmap(alloc->raw, alloc->alloc);
	assert(!_);

	alloc->raw = nil;
	alloc->pool = nil;
	alloc->alloc = 0;
}

void *ny_alloc_acquire(struct ny_alloc *restrict alloc) {
	assert(alloc);

	void *object = nil;

	/* Any objects remaining? */
	if (likely(alloc->free != UINT32_MAX)) {
		/* Pop object from list */
		object = index(alloc, alloc->free);
		alloc->free = *(uint32_t *) object;
	}

	return object;
}

void ny_alloc_release(struct ny_alloc *restrict alloc, void *restrict object) {
	assert(alloc);
	assert(object);

#ifdef NY_DEBUG_ALLOC
	/* Attempt to discover double-free situations */
	for (uint_least32_t iter = alloc->free; iter != UINT32_MAX;
		iter = *(uint32_t *) index(alloc, iter)) {
		assert(pointer(alloc, object) != iter);
	}
#endif

	/* Push object back onto list */
	*(uint32_t *) object = alloc->free;
	alloc->free = pointer(alloc, object);
}
