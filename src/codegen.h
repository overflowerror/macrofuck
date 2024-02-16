#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include "band.h"

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

#define move(t) _move_to(out, band, t)
#define move_to(r) move(r->start)
#define move_offset(r, o) move(r->start + o)

#define copy(s, t) _copy(out, band, s, t)
#define clone(s) _clone(out, band, s)

void _move_to(FILE*, band_t*, size_t);
void _copy(FILE*, band_t*, region_t*, region_t*);
region_t* _clone(FILE*, band_t*, region_t*);

int codegen(FILE*, struct program*);

#endif
