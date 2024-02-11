#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "alloc.h"

void* list_allocate(size_t element_size, size_t initial_capacity) {
	struct list_header* header = safe_alloc(sizeof(struct list_header) + initial_capacity * element_size);

	header->capacity = initial_capacity;
	header->length = 0;

	return header + 1;
}

struct list_header* list_header(void* ptr) {
	struct list_header* header = ptr;
	return header - 1;
}	

size_t list_size(void* ptr) {
	return list_header(ptr)->length;
}

void* list_ensure_space(void* ptr, size_t element_size, size_t additional) {
	struct list_header* header = list_header(ptr);

	if (header->capacity - header->length < additional) {
		header->capacity = header->length + additional;
		header = safe_realloc(header, sizeof(struct list_header) + element_size * header->capacity);
	}

	return header + 1;
}

void list_free(void* ptr) {
	free(list_header(ptr));
}

void list_remove(void* ptr, size_t element_size, size_t index) {
	struct list_header* header = list_header(ptr);

	if (header->length > 1) {
		memmove(
			ptr + index * element_size,
			ptr + (index + 1) * element_size,
			(header->length - index - 1) * element_size
		);
	}

	header->length--;
}
