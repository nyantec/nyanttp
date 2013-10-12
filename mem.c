#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/mman.h>

#include <defy/expect>
#include <defy/nil>
#include <defy/restrict>

#include <nyanttp/mem.h>
#include <nyanttp/util.h>

void *ny_mem_alloc(size_t length) {
	void *memory = nil;

#if NY_MEM_ALLOC_GUARD
	long pagesize = sysconf(_SC_PAGESIZE);
	assert(pagesize > 0);

	size_t alloc = ny_util_align(length, pagesize) + 2 * pagesize;
#else
	size_t alloc = length;
#endif

	void *base = mmap(nil, alloc, PROT_READ | PROT_WRITE,
		MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (unlikely(base == MAP_FAILED))
		goto exit;

#if NY_MEM_ALLOC_GUARD
	/* Setup lower guard page */
	if (unlikely(mprotect(base, pagesize, PROT_NONE)))
		goto unmap;

	/* Setup upper guard page */
	if (unlikely(mprotect((uint8_t *) base + (alloc - pagesize),
		pagesize, PROT_NONE)))
		goto unmap;

	memory = (uint8_t *) base + pagesize;
	goto exit;

unmap:;
	int save = errno;
	int _ = munmap(base, alloc);
	assert(!_);
	errno = save;
#else
	memory = base;
#endif

exit:
	return memory;
}

int ny_mem_free(void *restrict memory, size_t length) {
#if NY_MEM_ALLOC_GUARD
	long pagesize = sysconf(_SC_PAGESIZE);
	assert(pagesize > 0);

	void *base = (uint8_t *) memory - pagesize;
	size_t alloc = ny_util_align(length, pagesize) + 2 * pagesize;
#else
	void *base = memory;
	size_t alloc = length;
#endif

	return munmap(base, alloc);
}
