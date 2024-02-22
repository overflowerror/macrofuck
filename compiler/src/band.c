#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "band.h"
#include "list.h"
#include "alloc.h"

band_t* band_init(void) {
	band_t* band = safe_malloc(sizeof(band_t));
	band->position = 0;
	band->regions = list_new(region_t);
	band->band = list_new(region_t);
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

band_addr_t band_find_gap(band_t* band, size_t size) {
	size_t band_size = list_size(band->band);

	size_t current = 0;
	for (size_t i = 0; i < band_size; i++) {
		if (band->band[i] == NULL) {
			current++;
			if (current >= size) {
				return i + 1 - current;
			}
		} else {
			current = 0;
		}
	}

	size_t missing_size = size - current;

	band->band = list_ensure_space(band->band, sizeof(region_t*), missing_size);
	list_header(band->band)->length += missing_size;
	
	return band_size - current;
}

size_t band_find_empty_region_index(band_t* band) {
	size_t regions_size = list_size(band->regions);
	for (size_t i = 0; i < regions_size; i++) {
		if (band->regions[i] == NULL) {
			return i;
		}
	}

	list_add(band->regions, NULL);
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

region_t* band_allocate(band_t* band, size_t size) {
	region_t* region = band_allocate_region(band, size);
    region->name = NULL;
    region->is_temp = false;

	return region;
}

region_t* band_allocate_tmp(band_t* band, size_t size) {
	region_t* region = band_allocate_region(band, size);
	region->name = NULL;
	region->is_temp = true;

	return region;
}

void band_region_free(band_t* band, region_t* region) {
	band->regions[region->index] = NULL;
	for (size_t i = 0; i < region->size; i++) {
		band->band[region->start + i] = NULL;
	}

	free(region);
}

void band_region_free_raw(band_t* band, band_addr_t addr) {
	band_region_free(band, band->band[addr]);
}

size_t band_number_of_regions(band_t* band) {
    size_t regions_size = list_size(band->regions);

    size_t result = 0;
    for (size_t i = 0; i < regions_size; i++) {
        if (band->regions[i] != NULL) {
            result++;
        }
    }
    return result;
}

region_t** band_iterate(band_t* band, region_t** last_ptr) {
    size_t size = list_size(band->regions);

    size_t offset = 0;
    if (last_ptr) {
        offset = (((uintptr_t) last_ptr) - ((uintptr_t) band->regions)) / sizeof(void *) + 1;
    }

    for (size_t i = offset; i < size; i++) {
        if (band->regions[i] != NULL) {
            return band->regions + i;
        }
    }

    return NULL;
}
