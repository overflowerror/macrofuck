%{
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "error.h"
#include "ast.h"

int yylex(void);
void yyerror(const char*);

extern struct program* program;

%}

%union {
	struct program* program;
	struct statement* statement;
	struct expression* expr;
	long long number;
	char ch;
}

%type <program> stats
%type <statement> stat print
%type <expr> expr literal

%token <number> NUM 
%token <ch> CHAR
%token SEMICOLON
%token PRINT

%start file

%%

file: stats
		{
			program = $1;
		} 
;

stats:    /* empty */
		{
			$$ = program_new();
		}
	| stats stat SEMICOLON
		{
			$$ = $1;
			program_add_statement($$, $2);
		}
;

stat: print
;

print: PRINT expr
		{
			$$ = print_statement_new($2);
		}
;

expr: literal
;

literal:  NUM
		{
			yyerror(ERROR("number literals not yet supported"));
			YYERROR;
		}
	| CHAR
		{
			$$ = literal_expression_char_new($1);
		}

%%

void yyerror(const char* s) {
	extern int yylineno;
	fprintf(stderr, "%s (line %d)\n", s, yylineno);
}
