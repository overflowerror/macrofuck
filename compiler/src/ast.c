#include <stdlib.h>
#include <string.h>

#include <error.h>
#include <alloc.h>

#include "ast.h"

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

struct statement* declaration_statement_new(struct statement* assignment) {
    if (assignment->kind != ASSIGNMENT_STATEMENT) {
        panic("argument has to be assignment");
    }

    assignment->kind = DECL_STATEMENT;

    return assignment;
}


struct statement* assignment_statement_new(struct expression* variable, struct expression* expr) {
	_new(stat, statement);
	stat->kind = ASSIGNMENT_STATEMENT;
	stat->assignment = (struct assignment_statement) {
		.variable = variable,
		.value = expr,
	};
	return stat;
}

struct statement* expr_statement_new(struct expression* expr) {
    _new(stat, statement);
    stat->kind = EXPR_STATEMENT;
    stat->expr = (struct expr_statement) {
            .expr = expr,
    };
    return stat;
}

struct statement* if_statement_new(struct expression* condition, struct block* if_block, struct block* else_block) {
    _new(stat, statement);
    stat->kind = IF_STATEMENT;
    stat->if_else = (struct if_statement) {
            .condition = condition,
            .if_block = if_block,
            .else_block = else_block,
    };
    return stat;
}

struct statement* while_statement_new(struct expression* condition, struct block* block) {
    _new(stat, statement);
    stat->kind = WHILE_STATEMENT;
    stat->while_loop = (struct while_statement) {
            .condition = condition,
            .block = block,
    };
    return stat;
}

struct statement* map_statement_new(char* index_id, char* ref_id, char* list_id, struct block* block, bool reverse) {
    _new(stat, statement);
    stat->kind = MAP_STATEMENT;
    stat->map_loop = (struct map_statement) {
            .index_id = index_id,
            .ref_id = ref_id,
            .list_id = list_id,
            .block = block,
            .reverse = reverse,
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

struct expression* literal_expression_array_new(size_t length, struct expression** values) {
    if (values == NULL) {
        values = safe_malloc(length * sizeof(struct expression*));
        for (size_t i = 0; i < length; i++) {
            values[i] = literal_expression_num_new(0);
        }
    }

    _new(expr, expression);
    expr->kind = LITERAL;
    expr->type = UNKNOWN_TYPE;
    expr->literal = (struct literal_expression) {
        .kind = ARRAY_LITERAL,
        .array = (struct array_literal) {
            .length = length,
            .values = values,
        }
    };

    return expr;
}

struct expression* variable_expression_new(char* id) {
    _new(expr, expression);
    expr->kind = VARIABLE;
    expr->type = UNKNOWN_TYPE;
    expr->variable = (struct variable_expression) {
            .id = id,
            .is_offset = false,
            .offset = 0,
    };
    return expr;
}

struct expression* variable_expression_new_offset(char* id, size_t offset) {
	_new(expr, expression);
	expr->kind = VARIABLE;
	expr->type = UNKNOWN_TYPE;
	expr->variable = (struct variable_expression) {
		.id = id,
        .is_offset = true,
        .offset = offset,
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

struct expression* builtin_call_expression_new(void) {
    _new(expr, expression);
    expr->kind = BUILTIN_CALL;
    expr->type = UNKNOWN_TYPE;
    expr->builtin_call = (struct builtin_call_expression) {
        .id = NULL,
        .argument_number = 0,
        .arguments = NULL,
    };
    return expr;
}

void builtin_call_expression_add_argument(struct expression* builtin_call, struct expression* argument) {
    builtin_call->builtin_call.arguments = safe_realloc(builtin_call->builtin_call.arguments, (builtin_call->builtin_call.argument_number + 1) * sizeof(struct expression*));
    memmove(builtin_call->builtin_call.arguments + 1, builtin_call->builtin_call.arguments, builtin_call->builtin_call.argument_number * sizeof(struct expression*));
    builtin_call->builtin_call.arguments[0] = argument;
    builtin_call->builtin_call.argument_number++;
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
