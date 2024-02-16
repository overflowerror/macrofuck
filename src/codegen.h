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

#define move_to(t) _move_to(out, band, t)

#define add(v) { for (int i = 0; i < v; i++) inc(); }
#define sub(v) { for (int i = 0; i < v; i++) dec(); }

void _move_to(FILE*, band_t*, size_t);

int codegen(FILE*, struct program*);

#endif
