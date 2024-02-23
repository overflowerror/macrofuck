#include <stdlib.h>

#include "error.h"
#include "alloc.h"

void* safe_malloc(size_t length) {
	void* result = malloc(length);
	if (!result) {
        fprintf(stderr, "error while trying to allocate %zu bytes\n", length);
		panic("malloc");
	}
	return result;
}

void* safe_realloc(void* ptr, size_t length) {
	void* new = realloc(ptr, length);
	if (!new) {
        fprintf(stderr, "error while trying to allocate %zu bytes\n", length);
		panic("realloc");
	}
	return new;
}
