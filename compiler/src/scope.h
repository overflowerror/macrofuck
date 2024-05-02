#ifndef SCOPE_H
#define SCOPE_H

#include <dict.h>

#include "band.h"

typedef struct scope {
    struct scope* parent;
    band_t* band;
    dict_t* variables;
} scope_t;

scope_t* scope_init(band_t*);
scope_t* scope_new(scope_t*);

region_t* scope_add(scope_t*, const char*, size_t);
region_t* scope_existing(scope_t*, region_t*, const char*);
region_t* scope_add_tmp(scope_t*, size_t);
region_t* scope_add_ref(scope_t*, region_t*, size_t, size_t);

region_t* scope_get(scope_t*, const char*);

void scope_remove(scope_t*, region_t*);
void scope_free(scope_t*);

#endif
