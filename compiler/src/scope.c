#include <stdlib.h>
#include <stdbool.h>

#include "scope.h"
#include "band.h"
#include "alloc.h"
#include "dict.h"

scope_t* scope_init(band_t* band) {
    scope_t* root = scope_new(NULL);
    root->band = band;
    return root;
}

scope_t* scope_new(scope_t* parent) {
    scope_t* scope = safe_malloc(sizeof (scope_t));
    scope->parent = parent;
    if (parent) {
        scope->band = parent->band;
    }
    scope->variables = dict_new();

    return scope;
}

region_t* scope_add(scope_t* scope, const char* name, size_t size) {
    region_t* region = band_allocate(scope->band, size);
    region->name = name;
    dict_put(scope->variables, name, region);

    return region;
}

region_t* scope_existing(scope_t* scope, region_t* region, const char* name) {
    region->name = name;
    region->is_temp = false;

    dict_put(scope->variables, name, region);

    return region;
}

region_t* scope_add_tmp(scope_t* scope, size_t size) {
    region_t* region = band_allocate_tmp(scope->band, size);
    return region;
}

region_t* scope_get(scope_t* scope, const char* name) {
    region_t* region = dict_get(scope->variables, name);

    if (!region) {
        if (scope->parent) {
            return scope_get(scope->parent, name);
        } else {
            return NULL;
        }
    } else {
        return region;
    }
}

void scope_remove(scope_t* scope, region_t* region) {
    if (region->name) {
        dict_remove(scope->variables, region->name);
    }

    band_region_free(scope->band, region);
}

void scope_free(scope_t* scope) {
    struct dict_pair* current = NULL;
    while ((current = dict_iterate(scope->variables, current)) != NULL) {
        band_region_free(scope->band, current->ptr);
    }

    dict_free(scope->variables);

    free(scope);
}
