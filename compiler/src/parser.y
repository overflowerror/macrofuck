%{

#define YYDEBUG 1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <error.h>

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
%type <statement> stat definition assignment exprstat if while map
%type <expr> expr literal variable macroexpr builtincall noncalcexpr argumentlist arrayliteral
%type <expr> calcexpr1 calcexpr2 calcexpr3 calcexpr4 calcexpr5 calcexpr6 calcexpr7 calcexpr8

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
%token GREATER
%token LESS
%token GREATER_EQUAL
%token LESS_EQUAL
%token AND
%token OR
%token NOT

%token OPENING_BRACKETS
%token CLOSING_BRACKETS
%token OPENING_BRACES
%token CLOSING_BRACES
%token OPENING_SQ_BRACKETS
%token CLOSING_SQ_BRACKETS
%token COMMA

%token VAR
%token IF
%token ELSE
%token WHILE
%token MAP
%token RMAP
%token IN

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

stat: definition SEMICOLON
	| assignment SEMICOLON
	| exprstat SEMICOLON
	| if
	| while
	| map
;

definition: VAR assignment
		{
			$$ = declaration_statement_new($2);
		}
;

assignment: variable ASSIGNMENT expr
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
    | ELSE if
        {
            $$ = block_new();
            block_add_statement($$, $2);
        }
;

while: WHILE expr block
        {
            $$ = while_statement_new($2, $3);
        }
;

map: MAP ID COMMA ID IN ID block
        {
            $$ = map_statement_new($2, $4, $6, $7, false);
        }
    | RMAP ID COMMA ID IN ID block
        {
          $$ = map_statement_new($2, $4, $6, $7, true);
        }
;



block: OPENING_BRACES stats CLOSING_BRACES
        {
            $$ = $2;
        }
;

exprstat: expr
    {
        $$ = expr_statement_new($1);
    }
;

builtincall: ID OPENING_BRACKETS argumentlist CLOSING_BRACKETS
    {
        $$ = $3;
        $$->builtin_call.id = $1;
    }
;

argumentlist: /* empty */
                {
                    $$ = builtin_call_expression_new();
                }
            | expr
                {
                    $$ = builtin_call_expression_new();
                    builtin_call_expression_add_argument($$, $1);
                }
            | expr COMMA argumentlist
                {
                    $$ = $3;
                    builtin_call_expression_add_argument($$, $1);
                }
;

calcexpr1: calcexpr2
    | calcexpr1 OR calcexpr2
        {
            $$ = calc_expression_new($1, $3, DISJUNCTION);
        }
;

calcexpr2: calcexpr3
    | calcexpr2 AND calcexpr3
        {
            $$ = calc_expression_new($1, $3, CONJUNCTION);
        }
;

calcexpr3: calcexpr4
    | calcexpr3 EQUAL calcexpr4
        {
            $$ = calc_expression_new($1, $3, EQUALS);
        }
    | calcexpr3 NOT_EQUAL calcexpr4
        {
            $$ = calc_expression_new($1, $3, NOT_EQUALS);
        }
;

calcexpr4: calcexpr5
    | calcexpr4 LESS calcexpr5
        {
            $$ = calc_expression_new($1, $3, LESS_THAN);
        }
    | calcexpr4 GREATER calcexpr5
        {
            $$ = calc_expression_new($1, $3, GREATER_THAN);
        }
    | calcexpr4 LESS_EQUAL calcexpr5
        {
            $$ = calc_expression_new($1, $3, LESS_EQUALS);
        }
    | calcexpr4 GREATER_EQUAL calcexpr5
        {
            $$ = calc_expression_new($1, $3, GREATER_EQUALS);
        }
;

calcexpr5: calcexpr6
    | calcexpr5 PLUS calcexpr6
        {
            $$ = calc_expression_new($1, $3, ADDITION);
        }
    | calcexpr5 MINUS calcexpr6
        {
            $$ = calc_expression_new($1, $3, SUBTRACTION);
        }
;

calcexpr6: calcexpr7
    | calcexpr6 TIMES calcexpr7
        {
            $$ = calc_expression_new($1, $3, MULTIPLICATION);
        }
    | calcexpr6 DIVIDE calcexpr7
        {
            $$ = calc_expression_new($1, $3, DIVISION);
        }
    | calcexpr6 MOD calcexpr7
        {
            $$ = calc_expression_new($1, $3, MODULO);
        }
;

calcexpr7: calcexpr8
    | NOT calcexpr7
        {
            $$ = calc_expression_new($2, NULL, NEGATION);
        }
;

calcexpr8: noncalcexpr;

expr: noncalcexpr
    | calcexpr1
;

noncalcexpr: literal
    | variable
    | macroexpr
    | builtincall
	| OPENING_BRACKETS expr CLOSING_BRACKETS
	    {
	        $$ = $2;
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
	| arrayliteral
;

variable: ID
		{
			$$ = variable_expression_new($1);
		}
    | ID OPENING_SQ_BRACKETS NUM CLOSING_SQ_BRACKETS
        {
            $$ = variable_expression_new_offset($1, $3);
        }
;

arrayliteral: OPENING_SQ_BRACKETS NUM CLOSING_SQ_BRACKETS
                {
                    $$ = literal_expression_array_new($2, NULL);
                }
            | OPENING_SQ_BRACKETS CLOSING_SQ_BRACKETS OPENING_BRACES argumentlist CLOSING_BRACES
                {
                    $$ = literal_expression_array_new(
                        $4->builtin_call.argument_number,
                        $4->builtin_call.arguments
                    );
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
