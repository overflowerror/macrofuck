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
	int op; // hack, but C doesn't let me use forward declarations of enums
	long long number;
	char ch;
	char* str;
	char* id;
}

%type <program> stats
%type <statement> stat print definition macrostat
%type <expr> expr literal variable macroexpr calcexpr
%type <op> op

%token <number> NUM 
%token <ch> CHAR
%token <str> STR
%token <id> ID
%token <str> MACRO_CONTENT

%token SEMICOLON
%token ASSIGNMENT
%token PLUS
%token MINUS
%token TIMES
%token DIVIDE
%token MOD

%token OPENING_BRACKETS
%token CLOSING_BRACKETS
%token OPENING_BRACES
%token CLOSING_BRACES

%token VAR
%token PRINT
%token IF
%token ELSE

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
	| definition
	| if
	| macrostat
;

print: PRINT expr
		{
			$$ = print_statement_new($2);
		}
;

definition: VAR ID ASSIGNMENT expr
		{
			$$ = declaration_statement_new($2, $4);
		}
;

if: IF expr block optelse
;

optelse: /* empty */
    | ELSE block
;

block: OPENING_BRACES stats CLOSING_BRACES
;

macrostat: macroexpr
        {
            $$ = macro_statement_new($1);
        }
;

expr: 	  literal
	| variable
	| macroexpr
	| OPENING_BRACKETS expr CLOSING_BRACKETS
	    {
	        $$ = $2;
	    }
	| calcexpr
;

calcexpr: OPENING_BRACKETS expr op expr CLOSING_BRACKETS
        {
            $$ = calc_expression_new($2, $4, $3);
        }
;

op: PLUS
        {
            $$ = ADDITION;
        }
    | MINUS
        {
            $$ = SUBTRACTION;
        }
    | TIMES
        {
            $$ = MULTIPLICATION;
        }
    | DIVIDE
        {
            $$ = DIVISION;
        }
    | MOD
        {
            $$ = MODULO;
        }
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
