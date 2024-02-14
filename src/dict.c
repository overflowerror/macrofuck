#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "dict.h"
#include "alloc.h"
#include "list.h"

int hash_string(const char* str) {
	int result = 0;
	while (*str != '\0') {
		result = *str + 31 * result;
		str++;
	}
	return result;
}

dict_t* dict_new(void) {
	dict_t* dict = safe_malloc(sizeof(dict_t));
	dict->size = 0;
	for (size_t i = 0; i < NUMBER_OF_BUCKETS; i++) {
		dict->buckets[i] = list_new(struct dict_pair);
	}

	return dict;
}

struct dict_pair* dict_search_in_bucket(dict_bucket_t bucket, const char* key) {
	size_t bucket_size = list_size(bucket);
	for (size_t i = 0; i < bucket_size; i++) {
		if (strcmp(bucket[i].key, key) == 0) {
			return &(bucket[i]);
		}
	}

	return NULL;
}

dict_bucket_t dict_find_bucket(dict_t* dict, const char* key) {
	int hash = hash_string(key);
	return dict->buckets[hash % NUMBER_OF_BUCKETS];
}

struct dict_pair* dict_search(dict_t* dict, const char* key) {
	dict_bucket_t bucket = dict_find_bucket(dict, key);
	return dict_search_in_bucket(bucket, key);
}

void dict_put_pair(dict_t* dict, struct dict_pair pair) {
	dict_bucket_t bucket = dict_find_bucket(dict, pair.key);
	
	struct dict_pair* existing_entry = dict_search_in_bucket(bucket, pair.key);
	if (existing_entry) {
		*existing_entry = pair;
	} else {
		list_add(bucket, pair);
	}
}

void dict_put(dict_t* dict, const char* key, void* ptr) {
	struct dict_pair pair = {
		.key = key,
		.ptr = ptr,
	};
	dict_put_pair(dict, pair);
}

void dict_puti(dict_t* dict, const char* key, long long i) {
	struct dict_pair pair = {
		.key = key,
		.i = i,
	};
	dict_put_pair(dict, pair);
}

bool dict_has(dict_t* dict, const char* key) {
	return dict_search(dict, key) != NULL;
}

void* dict_get(dict_t* dict, const char* key) {
	struct dict_pair* pair = dict_search(dict, key);
	if (!pair) {
		return NULL;
	} else {
		return pair->ptr;
	}
}

long long dict_geti(dict_t* dict, const char* key) {
	struct dict_pair* pair = dict_search(dict, key);
	if (!pair) {
		return 0;
	} else {
		return pair->i;
	}
}

void dict_remove(dict_t* dict, const char* key) {
	dict_bucket_t bucket = dict_find_bucket(dict, key);
	
	size_t bucket_size = list_size(bucket);
	for (size_t i = 0; i < bucket_size; i++) {
		if (strcmp(bucket[i].key, key) == 0) {
			list_remove(bucket, sizeof(struct dict_pair), i);
			return;
		}
	}
}
