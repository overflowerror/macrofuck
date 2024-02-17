#include <stdlib.h>

#include "ast.h"
#include "error.h"
#include "alloc.h"

#define _new(n, t) struct t* n = safe_malloc(sizeof(struct t))

#define adjust_array(stru, field, t) {\
	stru->field = safe_realloc(stru->field, stru->length * sizeof(t)); \
}

#define last(stru, field) stru->field[stru->length - 1]


struct block* block_new(void) {
	_new(block, block);
    block->length = 0;
    block->statements = NULL;
	return block;
}

void block_add_statement(struct block* block, struct statement* statement) {
	block->length++;
	adjust_array(block, statements, struct statement*);
	last(block, statements) = statement;
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

struct statement* macro_statement_new(struct expression* expr) {
    _new(stat, statement);
    stat->kind = MACRO_STATEMENT;
    stat->macro = (struct macro_statement) {
        .expr = expr,
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

struct expression* literal_expression_num_new(long long i) {
    _new(expr, expression);
    expr->kind = LITERAL;
    expr->type = INTEGER;
    expr->literal = (struct literal_expression) {
            .kind = NUMBER_LITERAL,
            .number = i,
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

struct expression* macro_expression_new(char* id, char* arg) {
    _new(expr, expression);
    expr->kind = MACRO;
    expr->type = UNKNOWN_TYPE;
    expr->macro = (struct macro_expression) {
            .id = id,
            .argument = arg,
    };
    return expr;
}

struct expression* calc_expression_new(struct expression* operand1, struct expression* operand2, enum calc_operator operator) {
    _new(expr, expression);
    expr->kind = CALCULATION;
    expr->type = UNKNOWN_TYPE;
    expr->calc = (struct calc_expression) {
            .operand1 = operand1,
            .operand2 = operand2,
            .operator = operator,
    };
    return expr;
}
