#include <stdlib.h>

#include "error.h"
#include "alloc.h"

void* safe_malloc(size_t length) {
	void* result = malloc(length);
	if (!result) {
		panic("safe_malloc");
	}
	return result;
}

void* safe_realloc(void* ptr, size_t length) {
	void* new = realloc(ptr, length);
	if (!new) {
		panic("safe_realloc");
	}
	return new;
}
