#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

void _panic(const char* f, const char* s);
#define panic(s) _panic(__func__, s)

#define WARN(msg) ("\033[33mwarning:\033[0m " msg)
#define ERROR(msg) ("\033[31mwerror:\033[0m " msg)
#define INFO(msg) ("\033[36minfo:\033[0m " msg)

#define lex_panic(...) { \
	fprintf(stderr, "lexical error (line %d): ", yylineno); \
	fprintf(stderr, __VA_ARGS__); \
	exit(1); \
}

#endif
