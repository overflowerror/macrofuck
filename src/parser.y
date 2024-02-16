%{

#define YYDEBUG 1

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

%verbose

%union {
	struct program* program;
	struct statement* statement;
	struct expression* expr;
	long long number;
	char ch;
	char* str;
	char* id;
}

%type <program> stats
%type <statement> stat print definition macrostat
%type <expr> expr literal variable macroexpr

%token <number> NUM 
%token <ch> CHAR
%token <str> STR
%token <id> ID
%token <str> MACRO_CONTENT

%token SEMICOLON
%token EQUALS

%token VAR
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

stat:	  print
	| definition
	| macrostat
;

print: PRINT expr
		{
			$$ = print_statement_new($2);
		}
;

definition: VAR ID EQUALS expr
		{
			$$ = declaration_statement_new($2, $4);
		}
;

macrostat: macroexpr
        {
            $$ = macro_statement_new($1);
        }
;

expr: 	  literal
	| variable
	| macroexpr
;

literal:  NUM
		{
			$$ = literal_expression_num_new($1);
		}
	| CHAR
		{
			$$ = literal_expression_char_new($1);
		}
	| STR {
			$$ = literal_expression_str_new($1);
		}
;

variable: ID
		{
			$$ = variable_expression_new($1);
		}
;

macroexpr: ID MACRO_CONTENT
        {
            $$ = macro_expression_new($1, $2);
        }
;

%%

void yyerror(const char* s) {
	extern int yylineno;
	fprintf(stderr, "%s (line %d)\n", s, yylineno);
}
