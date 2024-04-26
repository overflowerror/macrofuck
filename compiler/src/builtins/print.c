#include <stdio.h>

#include <error.h>

#include "../scope.h"
#include "../codegen.h"

static size_t print_region(FILE* out, scope_t* scope, region_t* region) {
    move_to(region);

    output();

    size_t i = 1;
    for (; i < region->size; i++) {
        next();
        output();
    }
    scope->band->position += region->size - 1;

    if (region->is_temp)
        scope_remove(scope, region);

    return i;
}

extern region_t* print(FILE* out, scope_t* scope, size_t argc, region_t** argv) {
    if (argc < 1) {
        panic("print() needs at least one argument");
    }

    for (size_t i = 0; i < argc; i++) {
        print_region(out, scope, argv[i]);
    }

    // TODO: do something with this
    region_t* result = scope_add_tmp(scope, 1);
    return result;
}
