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

static void codegen_add_char(FILE* out, scope_t* scope, size_t position, char c) {
    move(position);
    reset();
    add(c);
}

static region_t* codegen_literal_expr(FILE* out, scope_t* scope, struct literal_expression expr) {
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
        case ARRAY_LITERAL:
            region = scope_add_tmp(scope, expr.array.length);
            for (size_t i = 0; i < expr.array.length; i++) {
                region_t* tmp = codegen_expr(out, scope, expr.array.values[i]);
                if (tmp->size > 1) {
                    panic("arrays can only contain scalar values");
                }
                _copy(out, scope, tmp, 0, region, i);
                if (tmp->is_temp) {
                    scope_remove(scope, tmp);
                }
            }
            break;
        default:
            fprintf(stderr, "literal kind: %d\n", expr.kind);
            panic("unknown literal kind");
    }
    return region;
}

static region_t* codegen_variable_expr(FILE* _, scope_t* scope, struct variable_expression expr) {
    region_t* region = scope_get(scope, expr.id);
    if (!region) {
        fprintf(stderr, "unknown variable: %s\n", expr.id);
        exit(1);
    }
    return region;
}

static region_t* codegen_macro_expr(FILE* out, scope_t* scope, struct macro_expression expr) {
    builtin_t macro = find_builtin(expr.id);
    if (macro == NULL) {
        panic("unable to find builtin");
    }

    region_t* arg = scope_get(scope, expr.argument);
    if (arg == NULL) {
        panic("argument has to be a variable");
    }

    return macro(out, scope, 1, &arg);
}

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

static region_t* codegen_addition(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    calc_prefix(true);

    move_to(op2);
    loop({
        dec();
        move_to(result); inc();
        move_to(op2);
    });

    calc_postfix();
}

static region_t* codegen_subtraction(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    calc_prefix(false);

    move_to(op2);
    loop({
        dec();
        move_to(result); dec();
        move_to(op2);
    });

    calc_postfix();
}

static region_t* codegen_multiplication(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
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

static region_t* codegen_division(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
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

static region_t* codegen_modulo(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
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

static region_t* codegen_equals(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
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

static region_t* codegen_not_equals(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
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

static region_t* codegen_conjunction(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
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

static region_t* codegen_disjunction(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
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

static region_t* codegen_greater_than(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    region_t* result = scope_add_tmp(scope, 1);
    move_to(result); reset();
    region_t* tmp = scope_add_tmp(scope, 1);
    region_t* op2_is_empty = scope_add_tmp(scope, 1);

    move_to(op1);
    loop({
        dec();
        move_to(op2_is_empty); reset(); inc();
        move_to(tmp); reset();
        move_to(op2);
        loop({
            move_to(tmp); inc();
            move_to(op2_is_empty); reset();
            move_to(op2); dec();
        });

        move_to(tmp);
        loop({
            move_to(op2); inc();
            move_to(tmp); dec();
        });

        move_to(op2); dec();

        move_to(op2_is_empty);
        loop({
            move_to(result); inc();
            move_to(op1); reset();
            move_to(op2_is_empty); dec();
        });

        move_to(op1);
    });

    scope_remove(scope, op1);
    scope_remove(scope, op2);
    scope_remove(scope, tmp);
    scope_remove(scope, op2_is_empty);

    return result;
}

static region_t* codegen_less_than(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    region_t* result = scope_add_tmp(scope, 1);
    move_to(result); reset();
    region_t* tmp = scope_add_tmp(scope, 1);
    region_t* op1_is_empty = scope_add_tmp(scope, 1);

    move_to(op2);
    loop({
        dec();
        move_to(op1_is_empty); reset(); inc();
        move_to(tmp); reset();
        move_to(op1);
        loop({
            move_to(tmp); inc();
            move_to(op1_is_empty); reset();
            move_to(op1); dec();
        });

        move_to(tmp);
        loop({
            move_to(op1); inc();
            move_to(tmp); dec();
        });

        move_to(op1); dec();

        move_to(op1_is_empty);
        loop({
            move_to(result); inc();
            move_to(op2); reset();
            move_to(op1_is_empty); dec();
        });

        move_to(op2);
    });

    scope_remove(scope, op1);
    scope_remove(scope, op2);
    scope_remove(scope, tmp);
    scope_remove(scope, op1_is_empty);

    return result;
}

static region_t* codegen_greater_than_or_equals(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    region_t* result = scope_add_tmp(scope, 1);
    move_to(result); reset(); inc();
    region_t* tmp = scope_add_tmp(scope, 1);
    region_t* op1_is_empty = scope_add_tmp(scope, 1);

    move_to(op2);
    loop({
        dec();
        move_to(op1_is_empty); reset(); inc();
        move_to(tmp); reset();
        move_to(op1);
        loop({
            move_to(tmp); inc();
            move_to(op1_is_empty); reset();
            move_to(op1); dec();
        });

        move_to(tmp);
        loop({
            move_to(op1); inc();
            move_to(tmp); dec();
        });

        move_to(op1); dec();

        move_to(op1_is_empty);
        loop({
            move_to(result); reset();
            move_to(op2); reset();
            move_to(op1_is_empty); dec();
        });

        move_to(op2);
    });

    scope_remove(scope, op1);
    scope_remove(scope, op2);
    scope_remove(scope, tmp);
    scope_remove(scope, op1_is_empty);

    return result;
}

static region_t* codegen_less_than_or_equals(FILE* out, scope_t* scope, region_t* op1, region_t* op2) {
    if (!op1->is_temp) {
        op1 = clone(op1);
    }
    if (!op2->is_temp) {
        op2 = clone(op2);
    }

    region_t* result = scope_add_tmp(scope, 1);
    move_to(result); reset(); inc();
    region_t* tmp = scope_add_tmp(scope, 1);
    region_t* op2_is_empty = scope_add_tmp(scope, 1);

    move_to(op1);
    loop({
        dec();
        move_to(op2_is_empty); reset(); inc();
        move_to(tmp); reset();
        move_to(op2);
        loop({
            move_to(tmp); inc();
            move_to(op2_is_empty); reset();
            move_to(op2); dec();
        });

        move_to(tmp);
        loop({
            move_to(op2); inc();
            move_to(tmp); dec();
        });

        move_to(op2); dec();

        move_to(op2_is_empty);
        loop({
            move_to(result); reset();
            move_to(op1); reset();
            move_to(op2_is_empty); dec();
        });

        move_to(op1);
    });

    scope_remove(scope, op1);
    scope_remove(scope, op2);
    scope_remove(scope, tmp);
    scope_remove(scope, op2_is_empty);

    return result;
}

static region_t* codegen_negation(FILE* out, scope_t* scope, region_t* op) {
    if (!op->is_temp) {
        op = clone(op);
    }

    region_t* result = scope_add_tmp(scope, 1);
    move_to(result); reset(); inc();

    move_to(op);
    loop({
        move_to(result); dec();
        move_to(op); reset();
    });

    scope_remove(scope, op);

    return result;
}

static region_t* codegen_calc_expr(FILE* out, scope_t* scope, struct calc_expression expr) {
    region_t* operand1 = codegen_expr(out, scope, expr.operand1);
    region_t* operand2 = NULL;
    if (expr.operand2) {
        operand2 = codegen_expr(out, scope, expr.operand2);
    }

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
        case GREATER_THAN:
            result = codegen_greater_than(out, scope, operand1, operand2);
            break;
        case LESS_THAN:
            result = codegen_less_than(out, scope, operand1, operand2);
            break;
        case GREATER_EQUALS:
            result = codegen_greater_than_or_equals(out, scope, operand1, operand2);
            break;
        case LESS_EQUALS:
            result = codegen_less_than_or_equals(out, scope, operand1, operand2);
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
        case NEGATION:
            result = codegen_negation(out, scope, operand1);
            break;
        default:
            fprintf(stderr, "unknown operator: %d\n", expr.operator);
            panic("unknown operator");
    }

    return result;
}

static region_t* codegen_builtin_expr(FILE* out, scope_t* scope, struct builtin_call_expression expr) {
    builtin_t builtin = find_builtin(expr.id);

    region_t** args = alloca(expr.argument_number * sizeof(region_t*));

    for (size_t i = 0; i < expr.argument_number; i++) {
        args[i] = codegen_expr(out, scope, expr.arguments[i]);
    }

    return builtin(out, scope, expr.argument_number, args);
}

region_t* codegen_expr(FILE* out, scope_t* scope, struct expression* expr) {
    switch(expr->kind) {
        case LITERAL:
            return codegen_literal_expr(out, scope, expr->literal);
        case VARIABLE:
            return codegen_variable_expr(out, scope, expr->variable);
        case MACRO:
            return codegen_macro_expr(out, scope, expr->macro);
        case BUILTIN_CALL:
            return codegen_builtin_expr(out, scope, expr->builtin_call);
        case CALCULATION:
            return codegen_calc_expr(out, scope, expr->calc);
        default:
            fprintf(stderr, "expression kind: %d\n", expr->kind);
            panic("unknown expression kind");
            return NULL;
    }
}
