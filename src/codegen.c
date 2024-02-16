#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "ast.h"
#include "band.h"
#include "error.h"


void _move_to(FILE* out, band_t* band, size_t target) {
	while (target > band->position) {
		next();
		band->position++;	
	}
	while (target < band->position) {
		prev();
		band->position--;
	}
}

static void region_used(band_t* band, region_t* region) {
	if (region->is_temp) {
		band_region_free(band, region);
	}
}

static void reset_position(FILE* out, band_t* band, band_addr_t position) {
	move_to(position);
	reset();
}

static void reset_region(FILE* out, band_t* band, region_t* region) {
	for (size_t i = 0; i < region->size; i++) {
		reset_position(out, band, region->start + i);
	}
}

void codegen_add_char(FILE* out, band_t* band, size_t position, char c) {
	move_to(position);
	reset();
	add(c);
}

region_t* codegen_literal_expr(FILE* out, band_t* band, struct literal_expression expr) {
	region_t* region = NULL;
	switch(expr.kind) {
        case NUMBER_LITERAL:
            if (expr.number > 255) {
                fprintf(stderr, "literal %lld greater than 255\n", expr.number);
                panic("number literal too big");
            }
            region = band_allocate_tmp(band, 1);
            move_to(region->start);
            reset(); add(expr.number);
            break;
		case CHAR_LITERAL:
			region = band_allocate_tmp(band, 1);
			codegen_add_char(out, band, region->start, expr.ch);
			break;
		case STRING_LITERAL: {
			size_t l = strlen(expr.str); // don't copy \0
			region = band_allocate_tmp(band, strlen(expr.str));
			for (size_t i = 0; i < l; i++) {
				codegen_add_char(out, band, region->start + i, expr.str[i]);
			}
			break;
		}
		default:
			fprintf(stderr, "literal kind: %d\n", expr.kind);
			panic("unknown literal kind");
	}
	return region; 
}

region_t* codegen_variable_expr(FILE* _, band_t* band, struct variable_expression expr) {
	region_t* region = band_region_for_var(band, expr.id);
	if (!region) {
		fprintf(stderr, "unknown variable: %s\n", expr.id);
		exit(1);
	}
	return region;
}

region_t* codegen_macro_expr(FILE* out, band_t* band, struct macro_expression expr) {
    // TODO
    return band_allocate_tmp(band, 1);
}

region_t* codegen_expr(FILE* out, band_t* band, struct expression* expr) {
	switch(expr->kind) {
		case LITERAL:
			return codegen_literal_expr(out, band, expr->literal);
		case VARIABLE:
			return codegen_variable_expr(out, band, expr->variable);
        case MACRO:
            return codegen_macro_expr(out, band, expr->macro);
		default:
			fprintf(stderr, "expression kind: %d\n", expr->kind);
			panic("unknown expression kind");
			return NULL;
	}
} 

void codegen_print_statement(FILE* out, band_t* band, struct print_statement statement) {
	region_t* region = codegen_expr(out, band, statement.value);
	move_to(region->start);

	output();
	for (size_t i = 1; i < region->size; i++) {
		next();
        output();
	}
	band->position += region->size - 1;

	region_used(band, region);
}

region_t* clone_region(FILE* out, band_t* band, region_t* original) {
	region_t* tmp = band_allocate_tmp(band, 1);
	region_t* clone = band_allocate_tmp(band, original->size);	

	reset_position(out, band, tmp->start);
	for (size_t i = 0; i < original->size; i++) {
		reset_position(out, band, clone->start + i);

		move_to(original->start + i);
        loop({
             dec();
             move_to(tmp->start);
             inc();
             move_to(clone->start + i);
             inc();
             move_to(original->start + i);
        });

        move_to(tmp->start);
        loop({
             dec();
             move_to(original->start + i);
             inc();
             move_to(tmp->start);
        });
	}

	band_region_free(band, tmp);
	return clone;
}

void codegen_decl_statement(FILE* out, band_t* band, struct declaration_statement statement) {
	region_t* region = codegen_expr(out, band, statement.value);

	if (!region->is_temp) {
		region = clone_region(out, band, region);		
	}

	band_make_var(band, region, statement.id);
}

void codegen_macro_statement(FILE* out, band_t* band, struct macro_statement statement) {
    region_t* region = codegen_expr(out, band, statement.expr);
    if (region->is_temp) {
        band_region_free(band, region);
    }
}

void codegen_statement(FILE* out, band_t* band, struct statement* statement) {
	switch(statement->kind) {
		case PRINT_STATEMENT:
			codegen_print_statement(out, band, statement->print);
			break;
		case DECL_STATEMENT:
			codegen_decl_statement(out, band, statement->decl);
			break;
        case MACRO_STATEMENT:
            codegen_macro_statement(out, band, statement->macro);
		default:
			fprintf(stderr, "statement kind: %d\n", statement->kind);
			panic("unknown statement kind");
	}
	// add newline after each statement
	fprintf(out, "\n");
}

int codegen(FILE* out, struct program* program) {
	band_t* band = band_init();
	
	for (size_t i = 0; i < program->length; i++) {
		codegen_statement(out, band, program->statements[i]);
	}

	return 0;
}
