#include <stdlib.h>

#include "ast.h"
#include "error.h"
#include "alloc.h"

#define _new(n, t) struct t* n = safe_malloc(sizeof(struct t))

#define adjust_array(stru, field, t) {\
	stru->field = safe_realloc(stru->field, stru->length * sizeof(t)); \
}

#define last(stru, field) stru->field[stru->length - 1]


struct program* program_new(void) {
	_new(program, program);
	program->length = 0;
	program->statements = NULL;
	return program;
}

void program_add_statement(struct program* program, struct statement* statement) {
	program->length++;
	adjust_array(program, statements, struct statement*);
	last(program, statements) = statement;
}

struct statement* print_statement_new(struct expression* expr) {	
	_new(stat, statement);
	stat->kind = PRINT_STATEMENT;
	stat->print = (struct print_statement) {
		.value = expr,
	};
	return stat;
}

struct statement* declaration_statement_new(char* id, struct expression* expr) {
	_new(stat, statement);
	stat->kind = DECL_STATEMENT;
	stat->decl = (struct declaration_statement) {
		.id = id,
		.value = expr,
	};
	return stat;
}

struct expression* literal_expression_char_new(char c) {
	_new(expr, expression);
	expr->kind = LITERAL;
	expr->type = CHARACTER;
	expr->literal = (struct literal_expression) {
		.kind = CHAR_LITERAL,
		.ch = c,
	};
	return expr;
}

struct expression* literal_expression_str_new(char* s) {
	_new(expr, expression);
	expr->kind = LITERAL;
	expr->type = STRING;
	expr->literal = (struct literal_expression) {
		.kind = STRING_LITERAL,
		.str = s,
	};
	return expr;
}

struct expression* variable_expression_new(char* id) {
	_new(expr, expression);
	expr->kind = VARIABLE;
	expr->type = UNKNOWN_TYPE;
	expr->variable = (struct variable_expression) {
		.id = id,
	};
	return expr;
}
