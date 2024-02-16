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

#define move(t) _move_to(out, band, t)
#define move_to(r) move(r->start)
#define move_offset(r, o) move(r->start + o)

#define add(v) { for (int _i = 0; _i < v; _i++) inc(); }
#define sub(v) { for (int _i = 0; _i < v; _i++) dec(); }

void _move_to(FILE*, band_t*, size_t);

int codegen(FILE*, struct program*);

#endif
