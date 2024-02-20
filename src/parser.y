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

extern struct block* program;

%}

%verbose

%union {
	struct block* block;
	struct statement* statement;
	struct expression* expr;
	int op; // hack, but C doesn't let me use forward declarations of enums
	long long number;
	char ch;
	char* str;
	char* id;
}

%type <block> stats optelse block
%type <statement> stat print definition assignment macrostat if while
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
%token EQUAL
%token NOT_EQUAL

%token OPENING_BRACKETS
%token CLOSING_BRACKETS
%token OPENING_BRACES
%token CLOSING_BRACES

%token VAR
%token PRINT
%token IF
%token ELSE
%token WHILE

%start file

%%

file: stats
		{
			program = $1;
		} 
;

stats:    /* empty */
		{
			$$ = block_new();
		}
	| stats stat
		{
			$$ = $1;
			block_add_statement($$, $2);
		}
;

stat: print SEMICOLON
	| definition SEMICOLON
	| assignment SEMICOLON
	| macrostat SEMICOLON
	| if
	| while
;

print: PRINT expr
		{
			$$ = print_statement_new($2);
		}
;

definition: VAR assignment
		{
			$$ = declaration_statement_new($2);
		}
;

assignment: ID ASSIGNMENT expr
		{
			$$ = assignment_statement_new($1, $3);
		}
;

if: IF expr block optelse
        {
            $$ = if_statement_new($2, $3, $4);
        }
;

optelse: /* empty */
        {
            $$ = NULL;
        }
    | ELSE block
        {
            $$ = $2;
        }
;

while: WHILE expr block
        {
            $$ = while_statement_new($2, $3);
        }
;


block: OPENING_BRACES stats CLOSING_BRACES
        {
            $$ = $2;
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
    | EQUAL
        {
            $$ = EQUALS;
        }
    | NOT_EQUAL
        {
            $$ = NOT_EQUALS;
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
