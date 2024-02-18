#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen.h"
#include "ast.h"
#include "band.h"
#include "error.h"
#include "plugins.h"


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

void _copy(FILE* out, band_t* band, region_t* source, region_t* target) {
    size_t size = source->size;
    if (target->size < size) {
        size = target->size;
    }

    region_t* tmp = band_allocate_tmp(band, size);
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

    band_region_free(band, tmp);
}

region_t* _clone(FILE* out, band_t* band, region_t* region) {
    region_t* clone = band_allocate_tmp(band, region->size);
    move_to(clone); reset();
    copy(region, clone);
    return clone;
}


static void region_used(band_t* band, region_t* region) {
	if (region->is_temp) {
		band_region_free(band, region);
	}
}

static void reset_position(FILE* out, band_t* band, band_addr_t position) {
	move(position);
	reset();
}

static void reset_region(FILE* out, band_t* band, region_t* region) {
	for (size_t i = 0; i < region->size; i++) {
		reset_position(out, band, region->start + i);
	}
}

void codegen_add_char(FILE* out, band_t* band, size_t position, char c) {
	move(position);
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
            move_to(region);
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
    macro_t macro = find_macro(expr.id);
    return macro(out, band, expr.argument);
}

region_t* codegen_expr(FILE*, band_t*, struct expression*);

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
    band_region_free(band, op2); \
    return result;

region_t* codegen_addition(FILE* out, band_t* band, region_t* op1, region_t* op2) {
    calc_prefix(true);

    move_to(op2);
    loop({
        dec();
        move_to(result); inc();
        move_to(op2);
    });

    calc_postfix();
}
region_t* codegen_subtraction(FILE* out, band_t* band, region_t* op1, region_t* op2) {
    calc_prefix(false);

    move_to(op2);
    loop({
        dec();
        move_to(result); dec();
        move_to(op2);
    });

    calc_postfix();
}
region_t* codegen_multiplication(FILE* out, band_t* band, region_t* op1, region_t* op2) {
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

    band_region_free(band, tmp);
    band_region_free(band, op1);

    calc_postfix();
}
region_t* codegen_division(FILE* out, band_t* band, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    region_t* result = band_allocate_tmp(band, 1);
    move_to(result); reset();

    region_t* tmp = band_allocate_tmp(band, 1);
    move_to(tmp); reset();
    copy(op2, tmp);

    region_t* tmp2 = band_allocate_tmp(band, 1);

    region_t* flag = band_allocate_tmp(band, 1);

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

    band_region_free(band, flag);
    band_region_free(band, tmp2);
    band_region_free(band, tmp);
    band_region_free(band, op1);
    band_region_free(band, op2);
    return result;
}
region_t* codegen_modulo(FILE* out, band_t* band, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    region_t* tmp = band_allocate_tmp(band, 1);
    move_to(tmp); reset();
    copy(op2, tmp);

    region_t* tmp2 = band_allocate_tmp(band, 1);

    region_t* flag = band_allocate_tmp(band, 1);

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

    band_region_free(band, flag);
    band_region_free(band, tmp2);
    band_region_free(band, tmp);
    band_region_free(band, op2);
    return op1;
}

region_t* codegen_calc_expr(FILE* out, band_t* band, struct calc_expression expr) {
    region_t* operand1 = codegen_expr(out, band, expr.operand1);
    region_t* operand2 = codegen_expr(out, band, expr.operand2);

    region_t* result;

    switch (expr.operator) {
        case ADDITION:
            result = codegen_addition(out, band, operand1, operand2);
            break;
        case SUBTRACTION:
            result = codegen_subtraction(out, band, operand1, operand2);
            break;
        case MULTIPLICATION:
            result = codegen_multiplication(out, band, operand1, operand2);
            break;
        case DIVISION:
            result = codegen_division(out, band, operand1, operand2);
            break;
        case MODULO:
            result = codegen_modulo(out, band, operand1, operand2);
            break;
        default:
            fprintf(stderr, "unknown operator: %d\n", expr.operator);
            panic("unknown operator");
    }

    return result;
}

region_t* codegen_expr(FILE* out, band_t* band, struct expression* expr) {
	switch(expr->kind) {
		case LITERAL:
			return codegen_literal_expr(out, band, expr->literal);
		case VARIABLE:
			return codegen_variable_expr(out, band, expr->variable);
        case MACRO:
            return codegen_macro_expr(out, band, expr->macro);
        case CALCULATION:
            return codegen_calc_expr(out, band, expr->calc);
		default:
			fprintf(stderr, "expression kind: %d\n", expr->kind);
			panic("unknown expression kind");
			return NULL;
	}
} 

void codegen_print_statement(FILE* out, band_t* band, struct print_statement statement) {
	region_t* region = codegen_expr(out, band, statement.value);
	move_to(region);

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

	band_region_free(band, tmp);
	return clone;
}

void codegen_assignment_statement(FILE* out, band_t* band, struct assignment_statement statement) {
    region_t* region = codegen_expr(out, band, statement.value);
    region_t* var = band_region_for_var(band, statement.id);
    if (!var) {
        fprintf(stderr, "variable not found: %s\n", statement.id);
        panic("unknown variable");
    }

    move_to(var); reset();
    copy(region, var);

    region_used(band, region);
}

void codegen_decl_statement(FILE* out, band_t* band, struct assignment_statement statement) {
    if (band_region_for_var(band, statement.id)) {
        fprintf(stderr, "variable exists: %s\n", statement.id);
        panic("variable exists");
    }

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

void codegen_block(FILE*, band_t*, struct block*);

void codegen_if_statement(FILE* out, band_t* band, struct if_statement statement) {
    region_t* condition = codegen_expr(out, band, statement.condition);
    if (!condition->is_temp) {
        condition = clone(condition);
    }

    region_t *inverse_condition = NULL;
    if (statement.else_block) {
        inverse_condition = band_allocate_tmp(band, 1);
        move_to(inverse_condition); reset(); inc();
    }

    move_to(condition);
    loop({
        if (inverse_condition) {
            move_to(inverse_condition); reset();
        }

        codegen_block(out, band, statement.if_block);

        move_to(condition); reset();
    });

    if (inverse_condition) {
        move_to(inverse_condition);
        loop({
            codegen_block(out, band, statement.else_block);

            move_to(inverse_condition); reset();
        });

        band_region_free(band, inverse_condition);
    }

    band_region_free(band, condition);
}

void codegen_statement(FILE* out, band_t* band, struct statement* statement) {
	switch(statement->kind) {
		case PRINT_STATEMENT:
			codegen_print_statement(out, band, statement->print);
			break;
		case DECL_STATEMENT:
			codegen_decl_statement(out, band, statement->assignment);
			break;
        case ASSIGNMENT_STATEMENT:
            codegen_assignment_statement(out, band, statement->assignment);
            break;
        case MACRO_STATEMENT:
            codegen_macro_statement(out, band, statement->macro);
            break;
        case IF_STATEMENT:
            codegen_if_statement(out, band, statement->if_else);
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
                fprintf(stderr, "variable %s\n", region->variable);
            }
        }
    }
}

void codegen_block(FILE* out, band_t* band, struct block* block) {
    if (block == NULL) {
        return;
    }

    for (size_t i = 0; i < block->length; i++) {
        codegen_statement(out, band, block->statements[i]);
    }
}

int codegen(FILE* out, struct block* program) {
	band_t* band = band_init();

    codegen_block(out, band, program);

    check_allocations(band);

	return 0;
}
