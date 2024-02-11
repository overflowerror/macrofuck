#include <stdbool.h>
#include <stdlib.h>

#include "band.h"
#include "list.h"
#include "dict.h"
#include "alloc.h"

band_t* band_init(void) {
	band_t* band = safe_malloc(sizeof(band_t));
	band->regions = list_new(region_t);
	band->variables = dict_new();
	band->addr_lookup = list_new(region_t);
	return band;
}

region_t* band_region_for_addr(band_t* band, band_addr_t addr) {
	size_t band_length = list_size(band->band);
	if (band_length >= addr) {
		return NULL;
	} else {
		return band->band[addr];
	}
}

region_t* band_region_for_var(band_t* band, const char* variable) {
	return dict_get(band->variables, variable);
}

band_addr_t band_find_gap(band_t* band, size_t size) {
	size_t band_size = list_size(band->band);

	size_t current = 0;
	for (size_t i = 0; i < band_size; i++) {
		if (band->band[i] == NULL) {
			current++;
			if (current >= size) {
				return i - current;
			}
		} else {
			current = 0;
		}
	}

	size_t missing_size = size - current;

	band->band = list_ensure_space(band->band, sizeof(region_t*), missing_size);
	
	return band_size - current;
}

size_t band_find_empty_region_index(band_t* band) {
	size_t regions_size = list_size(band->regions);
	for (size_t i = 0; i < regions_size; i++) {
		if (band->regions[i] == NULL) {
			return i;
		}
	}

	band->regions = list_ensure_space(band->regions, sizeof(region_t*), 1);
	return regions_size;
}

region_t* band_allocate_region(band_t* band, size_t size) {
	region_t* region = safe_malloc(sizeof(region_t));
	band_addr_t addr = band_find_gap(band, size);
	size_t regions_index = band_find_empty_region_index(band);

	region->start = addr;
	region->size = size;
	region->index = regions_index;

	band->regions[regions_index] = region;
	for (size_t i = 0; i < size; i++) {
		band->band[addr + i] = region;
	}

	return region;
}

region_t* band_allocate_var(band_t* band, size_t size, const char* variable) {
	region_t* region = band_allocate_region(band, size);

	region->variable = variable;
	region->is_temp = false;

	dict_put(band->variables, variable, region);

	return region;
}

region_t* band_allocate_tmp(band_t* band, size_t size) {
	region_t* region = band_allocate_region(band, size);

	region->variable = NULL;
	region->is_temp = true;

	return region;
}

void band_region_free(band_t* band, region_t* region) {
	band->regions[region->index] = NULL;
	for (size_t i = 0; i < region->size; i++) {
		band->band[region->start + i] = NULL;
	}

	if (region->variable) {
		dict_remove(band->variables, region->variable);
	}

	free(region);
}

void band_region_free_raw(band_t* band, band_addr_t addr) {
	band_free(band, band->band[addr]);
}
