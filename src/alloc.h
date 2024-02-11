#ifndef ALLOC_H
#define ALLOC_H

#include <stdlib.h>

void* safe_malloc(size_t);
void* safe_realloc(void*, size_t);

#endif
