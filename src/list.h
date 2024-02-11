#ifndef LIST_H
#define LIST_H

#include <stdlib.h>
#include <stdalign.h>

struct list_header {
	size_t capacity;
	alignas(16) size_t length;
}

#define LIST_INITIAL_CAPACITY (16)

#define list_new(T) allocate_list(sizeof(T), LIST_INITIAL_CAPACITY)

void* list_allocate(size_t, size_t);
size_t list_size(void*);

struct list_header* list_header(void*);

void* list_ensure_space(void*, size_t, size_t);

#define list_add(l, e) (\
		(l) = list_ensure_space(l, sizeof(e), 1);\
		(l)[list_size(l)] = e;\
		list_header(l)->length++;\
	)

void list_free(void*);

#endif
