#include <stdio.h>

#include "y.tab.h"

#include <error.h>

void push_file_to_yy_stack(FILE*);
int yylex(void);

extern char* yytext;
extern YYSTYPE yylval;

static void handle_include(void) {
    int token = yylex();

    if (token != STR) {
        fprintf(stderr, "preprocessor: include: file name expected, got: %s\n", yytext);
        panic("processor syntax error");
    }

    const char* filename = yylval.str;
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        panic("file not found");
    }

    push_file_to_yy_stack(file);
}

extern int preproc_lex(void) {
    int token = yylex();

    switch(token) {
        case PRE_INCLUDE:
            handle_include();
            token = yylex();
            break;
    }
    return token;
}

