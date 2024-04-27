#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdio.h>

#include "ast.h"
#include "band.h"
#include "scope.h"

#define next() fprintf(out, ">")
#define prev() fprintf(out, "<")
#define inc() fprintf(out, "+")
#define dec() fprintf(out, "-")
#define loop(b) fprintf(out, "["); b; fprintf(out, "]");
#define reset() fprintf(out, "[-]")
#define output() fprintf(out, ".")
#define input() fprintf(out, ",")

#define add(v) { for (int _i = 0; _i < v; _i++) inc(); }
#define sub(v) { for (int _i = 0; _i < v; _i++) dec(); }

#define move(t) _move_to(out, scope, t)
#define move_to(r) move(r->start)
#define move_offset(r, o) move(r->start + o)

#define copy(s, t) _copy(out, scope, s, t)
#define clone(s) _clone(out, scope, s)

#define reset_region(r) _reset_region(out, scope, r)

#define region_used(region) if (region->is_temp) { scope_remove(scope, region); }

void _move_to(FILE*, scope_t*, size_t);
void _copy(FILE*, scope_t*, region_t*, region_t*);
region_t* _clone(FILE*, scope_t*, region_t*);
void _reset_region(FILE*, scope_t*, region_t*);


int codegen(FILE*, struct block*);

region_t* codegen_expr(FILE*, scope_t*, struct expression*);
void codegen_statement(FILE*, scope_t*, struct statement*);
void codegen_block(FILE*, scope_t*, struct block*);

#endif
