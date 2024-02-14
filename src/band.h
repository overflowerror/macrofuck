#ifndef BAND_H
#define BAND_H

#include <stdbool.h>
#include <stdlib.h>

#include "list.h"
#include "dict.h"

typedef size_t band_addr_t;

typedef struct {
	band_addr_t start;
	size_t index;
	size_t size;
	const char* variable;
	bool is_temp;	
} region_t;

typedef struct {
	size_t position;
	region_t** regions;
	dict_t* variables;
	region_t** band;
} band_t;

band_t* band_init(void);

region_t* band_region_for_addr(band_t*, band_addr_t);
region_t* band_region_for_var(band_t*, const char*);

region_t* band_allocate_var(band_t*, size_t, const char*);
region_t* band_allocate_tmp(band_t*, size_t);

void band_region_free(band_t*, region_t*);
void band_region_free_raw(band_t*, band_addr_t);


#endif