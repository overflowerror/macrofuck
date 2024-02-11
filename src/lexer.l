whitespace	[\n\t\n ]

dec		([1-9][0-9]*)|"0"
oct		"0"[0-9]+
hex		"0x"[0-9a-fA-F]+
bin		"0b"[01]+

print		"print"

char		"'"."'"

%option noyywrap
%option nodefault
%option nounput
%option noinput
%option yylineno

%x COMMENT
%%

%{
#include <stdlib.h>
#include <stdbool.h>

#include "y.tab.h"
%}

<*>"/*"		{ BEGIN(COMMENT); }
<COMMENT>"*/"	{ BEGIN(INITIAL); }
<COMMENT>[^*\n]	{ }
<COMMENT>\n	{ }
<COMMENT>"*"[^*\/\n]*	{ }

<INITIAL>"//"[^\n]\n		{ /* single line comment */ }

<INITIAL>{print}		{ return PRINT; }
<INITIAL>";"			{ return SEMICOLON; }



<INITIAL>{hex}		{ yylval.number = strtol(yytext + 2, NULL, 16); return NUM; }
<INITIAL>{dec}		{ yylval.number = strtol(yytext, NULL, 10); return NUM; }
<INITIAL>{oct}		{ yylval.number = strtol(yytext + 1, NULL, 8); return NUM; }
<INITIAL>{bin}		{ yylval.number = strtol(yytext + 2, NULL, 2); return NUM; }
<INITIAL>{char}		{ yylval.ch = yytext[1]; return CHAR; }
<INITIAL>{whitespace}+	{ }

<INITIAL>.		{ fprintf(stderr, "lexical error in line %d.\n", yylineno); exit(1); }
