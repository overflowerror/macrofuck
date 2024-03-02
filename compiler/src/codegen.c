#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <error.h>

#include "codegen.h"
#include "ast.h"
#include "band.h"
#include "scope.h"
#include "plugins.h"


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


static void region_used(scope_t* scope, region_t* region) {
	if (region->is_temp) {
		scope_remove(scope, region);
	}
}

void codegen_add_char(FILE* out, scope_t* scope, size_t position, char c) {
	move(position);
	reset();
	add(c);
}

region_t* codegen_literal_expr(FILE* out, scope_t* scope, struct literal_expression expr) {
	region_t* region = NULL;
	switch(expr.kind) {
        case NUMBER_LITERAL:
            if (expr.number > 255) {
                fprintf(stderr, "literal %lld greater than 255\n", expr.number);
                panic("number literal too big");
            }
            region = scope_add_tmp(scope, 1);
            move_to(region);
            reset(); add(expr.number);
            break;
		case CHAR_LITERAL:
			region = scope_add_tmp(scope, 1);
			codegen_add_char(out, scope, region->start, expr.ch);
			break;
		case STRING_LITERAL: {
			size_t l = strlen(expr.str); // don't copy \0
			region = scope_add_tmp(scope, strlen(expr.str));
			for (size_t i = 0; i < l; i++) {
				codegen_add_char(out, scope, region->start + i, expr.str[i]);
			}
			break;
		}
		default:
			fprintf(stderr, "literal kind: %d\n", expr.kind);
			panic("unknown literal kind");
	}
	return region; 
}

region_t* codegen_variable_expr(FILE* _, scope_t* scope, struct variable_expression expr) {
	region_t* region = scope_get(scope, expr.id);
	if (!region) {
		fprintf(stderr, "unknown variable: %s\n", expr.id);
		exit(1);
	}
	return region;
}

region_t* codegen_macro_expr(FILE* out, scope_t* scope, struct macro_expression expr) {
    macro_t macro = find_macro(expr.id);
    return macro(out, scope, expr.argument);
}

region_t* codegen_expr(FILE*, scope_t*, struct expression*);

#define swap_or_clone(result, op1, op2, allow_swap) \
    if (op1->is_temp) { \
        result = op1; \
    } else if (op2->is_temp && allow_swap) { \
        result = op2; \
        op2 = op1; \
    } else { \
        result = clone(op1); \
    }

#define calc_prefix(allow_swap) \
    region_t* result; \
    swap_or_clone(result, op1, op2, allow_swap); \
    if (!op2->is_temp) { \
        op2 = clone(op2); \
    }

#define calc_postfix() \
    scope_remove(scope, op2); \
    return result;

region_t* codegen_addition(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    calc_prefix(true);

    move_to(op2);
    loop({
        dec();
        move_to(result); inc();
        move_to(op2);
    });

    calc_postfix();
}
region_t* codegen_subtraction(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    calc_prefix(false);

    move_to(op2);
    loop({
        dec();
        move_to(result); dec();
        move_to(op2);
    });

    calc_postfix();
}
region_t* codegen_multiplication(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    calc_prefix(true);

    // make sure to not change op1
    op1 = clone(op1);
    region_t* tmp = clone(op1);

    move_to(result); reset();

    move_to(op2);
    loop({
        dec();

        move_to(tmp);
        loop({
            dec();
            move_to(result); inc();
            move_to(tmp);
        });
        copy(op1, tmp);

        move_to(op2);
    });

    scope_remove(scope, tmp);
    scope_remove(scope, op1);

    calc_postfix();
}
region_t* codegen_division(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    region_t* result = scope_add_tmp(scope, 1);
    move_to(result); reset();

    region_t* tmp = scope_add_tmp(scope, 1);
    move_to(tmp); reset();
    copy(op2, tmp);

    region_t* tmp2 = scope_add_tmp(scope, 1);

    region_t* flag = scope_add_tmp(scope, 1);

    move_to(op1);
    loop({
        dec();

        move_to(tmp); dec();

        move_to(flag); reset(); inc();

        move_to(tmp2); reset();
        copy(tmp, tmp2);
        move_to(tmp2);
        loop({
            move_to(flag); reset();
            move_to(tmp2); reset();
        });

        move_to(flag);
        loop({
            move_to(result); inc();
            copy(op2, tmp);
            move_to(flag); reset();
        });

        move_to(op1);
    });

    scope_remove(scope, flag);
    scope_remove(scope, tmp2);
    scope_remove(scope, tmp);
    scope_remove(scope, op1);
    scope_remove(scope, op2);
    return result;
}
region_t* codegen_modulo(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    region_t* tmp = scope_add_tmp(scope, 1);
    move_to(tmp); reset();
    copy(op2, tmp);

    region_t* tmp2 = scope_add_tmp(scope, 1);

    region_t* flag = scope_add_tmp(scope, 1);

    move_to(op1);
    loop({
        dec();

        move_to(tmp); dec();

        move_to(flag); reset(); inc();

        move_to(tmp2); reset();
        copy(tmp, tmp2);
        move_to(tmp2);
        loop({
            move_to(flag); reset();
            move_to(tmp2); reset();
        });

        move_to(flag);
        loop({
            copy(op2, tmp);
            move_to(flag); reset();
        });

        move_to(op1);
    });

    move_to(tmp);
    loop({
        loop({
            dec();
            move_to(op2); dec();
            move_to(tmp);
        });

        move_to(op2);
        loop({
            dec();
            move_to(op1); inc();
            move_to(op2);
        });

        move_to(tmp);
    });

    scope_remove(scope, flag);
    scope_remove(scope, tmp2);
    scope_remove(scope, tmp);
    scope_remove(scope, op2);
    return op1;
}

region_t* codegen_equals(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    move_to(op2);
    loop({
        dec();
        move_to(op1); dec();
        move_to(op2);
    });
    inc();

    move_to(op1);
    loop({
        move_to(op2); dec();
        move_to(op1); reset();
    });

    scope_remove(scope, op1);
    return op2;
}

region_t* codegen_not_equals(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    move_to(op2);
    loop({
             dec();
             move_to(op1); dec();
             move_to(op2);
         });

    scope_remove(scope, op2);
    return op1;
}

region_t* codegen_conjunction(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    region_t* tmp = scope_add_tmp(scope, 1);
    move_to(tmp); reset();

    move_to(op1);
    loop({
       move_to(op2);
       loop({
           move_to(tmp); inc();
           move_to(op2); reset();
       });

       move_to(op1); reset();
    });

    scope_remove(scope, op1);
    scope_remove(scope, op2);

    return tmp;
}

region_t* codegen_disjunction(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    region_t* tmp = scope_add_tmp(scope, 1);
    move_to(tmp); reset();

    move_to(op1);
    loop({
        move_to(tmp); inc();
        move_to(op1); reset();
    });

    move_to(op2);
    loop({
        move_to(tmp); inc();
        move_to(op2); reset();
    });

    move_to(tmp);
    loop({
        move_to(op1); inc();
        move_to(tmp); reset();
    })

    scope_remove(scope, op2);
    scope_remove(scope, tmp);

    return op1;
}



region_t* codegen_calc_expr(FILE* out, scope_t* scope, struct calc_expression expr) {
    region_t* operand1 = codegen_expr(out, scope, expr.operand1);
    region_t* operand2 = codegen_expr(out, scope, expr.operand2);

    region_t* result = NULL;

    switch (expr.operator) {
        case ADDITION:
            result = codegen_addition(out, scope, operand1, operand2);
            break;
        case SUBTRACTION:
            result = codegen_subtraction(out, scope, operand1, operand2);
            break;
        case MULTIPLICATION:
            result = codegen_multiplication(out, scope, operand1, operand2);
            break;
        case DIVISION:
            result = codegen_division(out, scope, operand1, operand2);
            break;
        case MODULO:
            result = codegen_modulo(out, scope, operand1, operand2);
            break;
        case EQUALS:
            result = codegen_equals(out, scope, operand1, operand2);
            break;
        case NOT_EQUALS:
            result = codegen_not_equals(out, scope, operand1, operand2);
            break;
        case CONJUNCTION:
            result = codegen_conjunction(out, scope, operand1, operand2);
            break;
        case DISJUNCTION:
            result = codegen_disjunction(out, scope, operand1, operand2);
            break;
        default:
            fprintf(stderr, "unknown operator: %d\n", expr.operator);
            panic("unknown operator");
    }

    return result;
}

region_t* codegen_expr(FILE* out, scope_t* scope, struct expression* expr) {
	switch(expr->kind) {
		case LITERAL:
			return codegen_literal_expr(out, scope, expr->literal);
		case VARIABLE:
			return codegen_variable_expr(out, scope, expr->variable);
        case MACRO:
            return codegen_macro_expr(out, scope, expr->macro);
        case CALCULATION:
            return codegen_calc_expr(out, scope, expr->calc);
		default:
			fprintf(stderr, "expression kind: %d\n", expr->kind);
			panic("unknown expression kind");
			return NULL;
	}
} 

void codegen_print_statement(FILE* out, scope_t* scope, struct print_statement statement) {
	region_t* region = codegen_expr(out, scope, statement.value);
	move_to(region);

	output();
	for (size_t i = 1; i < region->size; i++) {
		next();
        output();
	}
    scope->band->position += region->size - 1;

	region_used(scope, region);
}

region_t* clone_region(FILE* out, scope_t* scope, region_t* original) {
	region_t* tmp = scope_add_tmp(scope, 1);
	region_t* clone = scope_add_tmp(scope, original->size);

	move_to(tmp); reset();
	for (size_t i = 0; i < original->size; i++) {
		move_offset(clone, i); reset();

		move_offset(original, i);
        loop({
             dec();
             move_to(tmp);
             inc();
             move_offset(clone, i);
             inc();
             move_offset(original, i);
        });

        move_to(tmp);
        loop({
             dec();
             move_offset(original, i);
             inc();
             move_to(tmp);
        });
	}

	scope_remove(scope, tmp);
	return clone;
}

void _reset_region(FILE* out, scope_t* scope, region_t* region) {
    for (size_t i = 0; i < region->size; i++) {
        move_offset(region, i); reset();
    }
}

void codegen_assignment_statement(FILE* out, scope_t* scope, struct assignment_statement statement) {
    region_t* region = codegen_expr(out, scope, statement.value);
    region_t* var = scope_get(scope, statement.id);
    if (!var) {
        fprintf(stderr, "variable not found: %s\n", statement.id);
        panic("unknown variable");
    }

    if (var->start != region->start) {
        reset_region(var);
        copy(region, var);
    }

    region_used(scope, region);
}

void codegen_decl_statement(FILE* out, scope_t* scope, struct assignment_statement statement) {
    if (scope_get(scope, statement.id)) {
        fprintf(stderr, "variable exists: %s\n", statement.id);
        panic("variable exists");
    }

	region_t* region = codegen_expr(out, scope, statement.value);

	if (!region->is_temp) {
		region = clone_region(out, scope, region);
	}

	scope_existing(scope, region, statement.id);
}

void codegen_macro_statement(FILE* out, scope_t* scope, struct macro_statement statement) {
    region_t* region = codegen_expr(out, scope, statement.expr);
    if (region->is_temp) {
        scope_remove(scope, region);
    }
}

void codegen_block(FILE*, scope_t*, struct block*);

void codegen_if_statement(FILE* out, scope_t* scope, struct if_statement statement) {
    region_t* condition = codegen_expr(out, scope, statement.condition);
    if (!condition->is_temp) {
        condition = clone(condition);
    }

    region_t *inverse_condition = NULL;
    if (statement.else_block) {
        inverse_condition = scope_add_tmp(scope, 1);
        move_to(inverse_condition); reset(); inc();
    }

    move_to(condition);
    loop({
        if (inverse_condition) {
            move_to(inverse_condition); reset();
        }

        codegen_block(out, scope, statement.if_block);

        move_to(condition); reset();
    });

    if (inverse_condition) {
        move_to(inverse_condition);
        loop({
            codegen_block(out, scope, statement.else_block);

            move_to(inverse_condition); reset();
        });

        scope_remove(scope, inverse_condition);
    }

    scope_remove(scope, condition);
}

void codegen_while_statement(FILE* out, scope_t* scope, struct while_statement statement) {
    region_t* condition = scope_add_tmp(scope, 1);
    move_to(condition); reset();

    region_t* tmp = codegen_expr(out, scope, statement.condition);
    copy(tmp, condition);
    region_used(scope, tmp);

    move_to(condition);
    loop({
        codegen_block(out, scope, statement.block);

        tmp = codegen_expr(out, scope, statement.condition);
        move_to(condition); reset();
        copy(tmp, condition);

        region_used(scope, tmp);

        move_to(condition);
    });

    scope_remove(scope, condition);
}

void codegen_statement(FILE* out, scope_t* scope, struct statement* statement) {
	switch(statement->kind) {
		case PRINT_STATEMENT:
			codegen_print_statement(out, scope, statement->print);
			break;
		case DECL_STATEMENT:
			codegen_decl_statement(out, scope, statement->assignment);
			break;
        case ASSIGNMENT_STATEMENT:
            codegen_assignment_statement(out, scope, statement->assignment);
            break;
        case MACRO_STATEMENT:
            codegen_macro_statement(out, scope, statement->macro);
            break;
        case IF_STATEMENT:
            codegen_if_statement(out, scope, statement->if_else);
            break;
        case WHILE_STATEMENT:
            codegen_while_statement(out, scope, statement->while_loop);
            break;
		default:
			fprintf(stderr, "statement kind: %d\n", statement->kind);
			panic("unknown statement kind");
	}
	// add newline after each statement
	fprintf(out, "\n");
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
