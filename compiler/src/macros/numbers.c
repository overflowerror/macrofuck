#include <stdio.h>

#include <error.h>

#include "../scope.h"
#include "../codegen.h"

extern region_t* to_str(FILE* out, scope_t* scope, const char* _arg) {
    region_t* arg = scope_get(scope, _arg);
    if (!arg) {
        panic("argument has to be a variable");
    }

    region_t* str = scope_add_tmp(scope, 3);
    for (size_t i = 0; i < str->size; i++) {
        move_offset(str, i); reset();
    }

    region_t* copy = scope_add_tmp(scope, 1);
    move_to(copy); reset();

    region_t* ten = scope_add_tmp(scope, 1);
    move_to(ten); reset(); add(10);

    region_t* tmp = scope_add_tmp(scope, 1);
    move_to(tmp); reset();
    region_t* tmp2 = scope_add_tmp(scope, 1);
    move_to(tmp2); reset();
    region_t* tmp3 = scope_add_tmp(scope, 1);

    move_to(arg);
    loop({
        dec();
        move_to(tmp); inc();
        move_to(copy); inc();
        move_offset(str, 2); inc();
        move_offset(str, 1); reset();
        move_offset(str, 0); reset();
        move_to(ten); dec();
        loop({
            dec();
            move_offset(str, 0); inc();
            move_offset(str, 1); inc();
            move_to(ten);
        });
        move_offset(str, 1);
        loop({
            move_to(tmp); reset();
            move_offset(str, 1); reset();
        });
        move_offset(str, 0);
        loop({
            dec();
            move_to(ten); inc();
            move_offset(str, 0);
        });
        move_to(tmp);
        loop({
            move_to(tmp2); inc();
            move_offset(str,  2); reset();
            move_to(ten); add(10);
            move_to(tmp); reset();
        });

        move_to(arg);
    });

    move_to(copy);
    loop({
        dec();
        move_to(arg); inc();
        move_to(copy);
    });

    move_to(ten); reset(); add(10);

    move_to(tmp2);
    loop({
        dec();
        move_to(tmp); reset(); inc();
        move_offset(str, 1); inc();
        move_to(copy); reset();
        move_to(tmp3); reset();
        move_to(ten); dec();
        loop({
            dec();
            move_to(copy); inc();
            move_to(tmp3); inc();
            move_to(ten);
        });
        move_to(tmp3);
        loop({
            move_to(tmp); reset();
            move_to(tmp3); reset();
        });
        move_to(copy);
        loop({
            dec();
            move_to(ten); inc();
            move_to(copy);
        });

        move_to(tmp);
        loop({
            move_to(ten); add(10);
            move_offset(str, 0); inc();
            move_offset(str, 1); reset();
            move_to(tmp); reset();
        });

        move_to(tmp2);
    });

    for (size_t i = 0; i < str->size; i++) {
        move_offset(str, i); add('0');
    }

    scope_remove(scope, tmp3);
    scope_remove(scope, tmp2);
    scope_remove(scope, tmp);
    scope_remove(scope, ten);
    scope_remove(scope, copy);

    return str;
}
