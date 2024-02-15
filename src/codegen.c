#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "ast.h"
#include "band.h"
#include "error.h"


static void move_to(FILE* out, band_t* band, size_t target) {
	while (target > band->position) {
		fprintf(out, ">");
		band->position++;	
	}
	while (target < band->position) {
		fprintf(out, "<");
		band->position--;
	}
}

static void region_used(band_t* band, region_t* region) {
	if (region->is_temp) {
		band_region_free(band, region);
	}
}

static inline void print_repeat(FILE* out, size_t n, char* s) {
	for (size_t i = 0; i < n; i++) {
		fprintf(out, "%s", s);
	}
}

static void reset_position(FILE* out, band_t* band, band_addr_t position) {
	move_to(out, band, position);
	fprintf(out, "[-]");
}

static void reset_region(FILE* out, band_t* band, region_t* region) {
	for (size_t i = 0; i < region->size; i++) {
		reset_position(out, band, region->start + i);
	}
}

void codegen_add_char(FILE* out, band_t* band, size_t position, char c) {
	move_to(out, band, position);
	fprintf(out, "[-]");
	print_repeat(out, c, "+");
}

region_t* codegen_literal_expr(FILE* out, band_t* band, struct literal_expression expr) {
	region_t* region = NULL;
	switch(expr.kind) {
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

region_t* codegen_variable_expr(FILE* out, band_t* band, struct variable_expression expr) {
	region_t* region = band_region_for_var(band, expr.id);
	if (!region) {
		fprintf(stderr, "unknown variable: %s\n", expr.id);
		exit(1);
	}
	return region;
}

region_t* codegen_expr(FILE* out, band_t* band, struct expression* expr) {
	switch(expr->kind) {
		case LITERAL:
			return codegen_literal_expr(out, band, expr->literal);
			break;
		case VARIABLE:
			return codegen_variable_expr(out, band, expr->variable);
			break;
		default:
			fprintf(stderr, "expression kind: %d\n", expr->kind);
			panic("unknown expression kind");
			return NULL;
	}
} 

void codegen_print_statement(FILE* out, band_t* band, struct print_statement statement) {
	region_t* region = codegen_expr(out, band, statement.value);
	move_to(out, band, region->start);

	fprintf(out, ".");
	for (size_t i = 1; i < region->size; i++) {
		fprintf(out, ">.");
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

		move_to(out, band, original->start + i);
		fprintf(out, "[");
		fprintf(out, "-");
		move_to(out, band, tmp->start);
		fprintf(out, "+");
		move_to(out, band, clone->start + i);
		fprintf(out, "+");
		move_to(out, band, original->start + i);
		fprintf(out, "]");

		move_to(out, band, tmp->start);
		fprintf(out, "[");
		fprintf(out, "-");
		move_to(out, band, original->start + i);
		fprintf(out, "+");
		move_to(out, band, tmp->start);
		fprintf(out, "]");
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

void codegen_statement(FILE* out, band_t* band, struct statement* statement) {
	switch(statement->kind) {
		case PRINT_STATEMENT:
			codegen_print_statement(out, band, statement->print);
			break;
		case DECL_STATEMENT:
			codegen_decl_statement(out, band, statement->decl);
			break;
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
