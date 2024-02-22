#ifndef BAND_H
#define BAND_H

#include <stdbool.h>
#include <stdlib.h>

#include "list.h"

typedef size_t band_addr_t;

typedef struct {
	band_addr_t start;
	size_t index;
	size_t size;
	const char* name;
	bool is_temp;	
} region_t;

typedef struct {
	size_t position;
	region_t** regions;
	region_t** band;
} band_t;

band_t* band_init(void);

region_t* band_region_for_addr(band_t*, band_addr_t);

region_t* band_allocate(band_t*, size_t);
region_t* band_allocate_tmp(band_t*, size_t);

void band_region_free(band_t*, region_t*);
void band_region_free_raw(band_t*, band_addr_t);

size_t band_number_of_regions(band_t*);
region_t** band_iterate(band_t*, region_t**);

#endif
