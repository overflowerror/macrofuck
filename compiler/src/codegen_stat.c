#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>

#include <error.h>

#include "codegen.h"
#include "ast.h"
#include "band.h"
#include "scope.h"


static void codegen_assignment_statement(FILE* out, scope_t* scope, struct assignment_statement statement) {
    region_t* value = codegen_expr(out, scope, statement.value);
    region_t* var = codegen_expr(out, scope, statement.variable);

    if (statement.variable->kind != VARIABLE) {
        fprintf(stderr, "assigment lhs is not a variable (%d)\n", statement.variable->kind);
        panic("not a variable");
    }

    if (var->start != value->start) {
        reset_region(var);
        copy(value, var);
    }

    region_used(value);

    if (statement.variable->variable.is_offset) {
        // lhs is offset -> free ref
        scope_remove(scope, var);
    }
}

// only used by decl_statement
// -> TODO refactor to clone macro
static region_t* clone_region(FILE* out, scope_t* scope, region_t* original) {
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

static void codegen_decl_statement(FILE* out, scope_t* scope, struct assignment_statement statement) {
    if (statement.variable->kind != VARIABLE || statement.variable->variable.is_offset) {
        fprintf(stderr, "decl lhs is not a variable (%d)\n", statement.variable->kind);
        panic("not a variable");
    }

    if (scope_get(scope, statement.variable->variable.id)) {
        fprintf(stderr, "variable exists: %s\n", statement.variable->variable.id);
        panic("variable exists");
    }

    region_t* region = codegen_expr(out, scope, statement.value);

    if (!region->is_temp) {
        region = clone_region(out, scope, region);
    }

    scope_existing(scope, region, statement.variable->variable.id);
}

static void codegen_if_statement(FILE* out, scope_t* scope, struct if_statement statement) {
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

static void codegen_while_statement(FILE* out, scope_t* scope, struct while_statement statement) {
    region_t* condition = scope_add_tmp(scope, 1);
    move_to(condition); reset();

    region_t* tmp = codegen_expr(out, scope, statement.condition);
    copy(tmp, condition);
    region_used(tmp);

    move_to(condition);
    loop({
        codegen_block(out, scope, statement.block);

        tmp = codegen_expr(out, scope, statement.condition);
        move_to(condition); reset();
        copy(tmp, condition);

        region_used(tmp);

        move_to(condition);
    });

    scope_remove(scope, condition);
}


static void codegen_map_statement(FILE* out, scope_t* scope, struct map_statement statement) {
    // we need a local scope for index and value
    scope_t* local_scope = scope_new(scope);

    region_t* list = scope_get(scope, statement.list_id);
    if (!list) {
        fprintf(stderr, "variable not found: %s\n", statement.list_id);
        panic("variable not found");
    }

    region_t* index = scope_add(local_scope, statement.index_id, 1);
    move_to(index); reset();

    region_t* value = scope_add_ref(local_scope, list, 0, 1);
    scope_existing(local_scope, value, statement.ref_id);

    for (size_t i = 0; i < list->size; i++) {
        codegen_block(out, local_scope, statement.block);
        value->start++;
        move_to(index); inc();
    }

    scope_free(local_scope);
}


static void codegen_expr_statement(FILE* out, scope_t* scope, struct expr_statement statement) {
    region_t* region = codegen_expr(out, scope, statement.expr);
    if (region->is_temp) {
        scope_remove(scope, region);
    }
}

void codegen_statement(FILE* out, scope_t* scope, struct statement* statement) {
    switch(statement->kind) {
        case DECL_STATEMENT:
            codegen_decl_statement(out, scope, statement->assignment);
            break;
        case ASSIGNMENT_STATEMENT:
            codegen_assignment_statement(out, scope, statement->assignment);
            break;
        case EXPR_STATEMENT:
            codegen_expr_statement(out, scope, statement->expr);
            break;
        case IF_STATEMENT:
            codegen_if_statement(out, scope, statement->if_else);
            break;
        case WHILE_STATEMENT:
            codegen_while_statement(out, scope, statement->while_loop);
            break;
        case MAP_STATEMENT:
            codegen_map_statement(out, scope, statement->map_loop);
            break;
        default:
            fprintf(stderr, "statement kind: %d\n", statement->kind);
            panic("unknown statement kind");
    }
    // add newline after each statement
    fprintf(out, "\n");
}
