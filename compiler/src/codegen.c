#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <error.h>

#include "codegen.h"
#include "ast.h"
#include "band.h"
#include "scope.h"
#include "builtins/builtins.h"


void _move_to(FILE* out, scope_t* scope, size_t target) {
	while (target > scope->band->position) {
		next();
		scope->band->position++;
	}
	while (target < scope->band->position) {
		prev();
		scope->band->position--;
	}
}

void _copy(FILE* out, scope_t* scope, region_t* source, region_t* target) {
    size_t size = source->size;
    if (target->size < size) {
        size = target->size;
    }

    region_t* tmp = scope_add_tmp(scope, size);
    move_to(tmp); reset();

    for (size_t i = 0; i < size; i++) {
        move_offset(source, i);
        loop({
            dec();
            move_offset(target, i); inc();
            move_offset(tmp, i); inc();
            move_offset(source, i);
        });
    }

    for (size_t i = 0; i < size; i++) {
        move_offset(tmp, i);
        loop({
            dec();
            move_offset(source, i); inc();
            move_offset(tmp, i);
        });
    }

    scope_remove(scope, tmp);
}

region_t* _clone(FILE* out, scope_t* scope, region_t* region) {
    region_t* clone = scope_add_tmp(scope, region->size);
    move_to(clone); reset();
    copy(region, clone);
    return clone;
}

void _reset_region(FILE* out, scope_t* scope, region_t* region) {
    for (size_t i = 0; i < region->size; i++) {
        move_offset(region, i); reset();
    }
}

void check_allocations(band_t* band) {
    size_t number_of_allocated_regions = band_number_of_regions(band);

    if (number_of_allocated_regions > 0) {
        fprintf(stderr, WARN("code generation existed with allocated regions:\n"));

        region_t** region_ptr = NULL;
        while((region_ptr = band_iterate(band, region_ptr)) != NULL) {
            region_t* region = *region_ptr;

            fprintf(stderr, "- (%zu) ", region->size);
            if (region->is_temp) {
                fprintf(stderr, "[anonymous] (this is a bug in the compiler; %zu)\n", region->start);
            } else {
                fprintf(stderr, "variable %s\n", region->name);
            }
        }
    }
}

void codegen_block(FILE* out, scope_t* parent, struct block* block) {
    scope_t* scope = scope_new(parent);

    if (block == NULL) {
        return;
    }

    for (size_t i = 0; i < block->length; i++) {
        codegen_statement(out, scope, block->statements[i]);
    }

    scope_free(scope);
}

int codegen(FILE* out, struct block* program) {
	band_t* band = band_init();
    scope_t* global = scope_init(band);

    codegen_block(out, global, program);

    scope_free(global);
    check_allocations(band);

	return 0;
}
