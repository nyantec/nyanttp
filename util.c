/**
 * \file
 *
 * \internal
 */

#include "config.h"

#include <stddef.h>

#include <nyanttp/util.h>

size_t align(size_t size, size_t align) {
	return (size + align - (size_t) 1) & ~(align - (size_t) 1);
}
