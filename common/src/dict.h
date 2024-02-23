#ifndef DICT_H
#define DICT_H

#include <stdbool.h>

#include "list.h"

#define NUMBER_OF_BUCKETS (64)

struct dict_pair {
	const char* key;
	union {
		long long i;
		void* ptr;	
	};
};

typedef struct dict_pair* dict_bucket_t;

typedef struct {
	dict_bucket_t buckets[NUMBER_OF_BUCKETS];
	size_t size;	
} dict_t;

dict_t* dict_new(void);

void dict_put(dict_t*, const char*, void*);
void dict_puti(dict_t*, const char*, long long);

bool dict_has(dict_t*, const char*);

void* dict_get(dict_t*, const char*);
long long dict_geti(dict_t*, const char*);

void dict_remove(dict_t*, const char*);

struct dict_pair* dict_iterate(dict_t*, struct dict_pair*);

void dict_free(dict_t*);

#endif
