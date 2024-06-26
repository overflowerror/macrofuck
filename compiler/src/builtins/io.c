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

extern region_t* read_char(FILE* out, scope_t* scope, size_t argc, region_t** argv) {
    if (argc != 0) {
        panic("read_char() doesn't accept arguments");
    }

    region_t* result = scope_add_tmp(scope, 1);
    move_to(result); input();

    return result;
}

extern region_t* read(FILE* out, scope_t* scope, size_t argc, region_t** argv) {
    if (argc == 0) {
        // default to read_char
        return read_char(out, scope, argc, argv);
    }
    if (argc != 1) {
        panic("read() has at most one argument");
    }

    region_t* region = argv[0];

    for (size_t i = 0; i < region->size; i++) {
        move_offset(region, i); input();
    }

    return region;
}

