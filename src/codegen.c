#include <stdio.h>
#include <stdlib.h>

#include "codegen.h"
#include "ast.h"
#include "band.h"
#include "error.h"


static void move_to(FILE* out, band_t* band, size_t target) {
	while (target < band->position) {
		fprintf(out, ">");
		band->position++;	
	}
	while (target > band->position) {
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

static void reset_region(FILE* out, band_t* band, region_t* region) {
	// TODO
}

region_t* codegen_literal_expr(FILE* out, band_t* band, struct literal_expression expr) {
	region_t* region = NULL;
	switch(expr.kind) {
		case CHAR_LITERAL:
			region = band_allocate_tmp(band, 1);
			move_to(out, band, region->start);
			print_repeat(out, expr.ch, "+");
			break;
		default:
			fprintf(stderr, "literal kind: %d\n", expr.kind);
			panic("unknown literal kind");
	}
	return region; 
}

region_t* codegen_expr(FILE* out, band_t* band, struct expression* expr) {
	switch(expr->kind) {
		case LITERAL:
			return codegen_literal_expr(out, band, expr->literal);
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
	band->position += region->size;

	region_used(band, region);
}

void codegen_statement(FILE* out, band_t* band, struct statement* statement) {
	switch(statement->kind) {
		case PRINT_STATEMENT:
			codegen_print_statement(out, band, statement->print);
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
