whitespace	[\n\t\n ]

dec		([1-9][0-9]*)|"0"
oct		"0"[0-9]+
hex		"0x"[0-9a-fA-F]+
bin		"0b"[01]+

print		"print"
var		"var"

char		"'"."'"
id		[a-zA-Z_][a-zA-Z0-9_]*

%option noyywrap
%option nodefault
%option nounput
%option noinput
%option yylineno

%x COMMENT
%x STRING
%x MACRO

%{

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "y.tab.h"

#include "strbuf.h"
#include "error.h"

%}

%%

%{

strbuf_t strbuf = NULL;

%}

<INITIAL>"/*"	{ BEGIN(COMMENT); }
<COMMENT>"*/"	{ BEGIN(INITIAL); }
<COMMENT>[^*\n]	{ }
<COMMENT>\n	{ }
<COMMENT>"*"[^*\/\n]*	{ }

<INITIAL>"//"[^\n]\n		{ /* single line comment */ }

<INITIAL>";"			{ return SEMICOLON; }

<INITIAL>\"		{ BEGIN STRING; strbuf_clear(strbuf); }
<STRING>[^\\"\n]*	{ strbuf_append(strbuf, yytext); }
<STRING>\\n		{ strbuf_append_c(strbuf, '\n'); }
<STRING>\\t		{ strbuf_append_c(strbuf, '\t'); }
<STRING>\\[0-7]*	{ strbuf_append_c(strbuf, strtol(yytext+1, 0, 8)); }
<STRING>\\[\\"]		{ strbuf_append_c(strbuf, yytext[1]); }
<STRING>\"		{ yylval.str = strdup(strbuf); BEGIN INITIAL; return STR; }
<STRING>\\.		{ lex_panic("invalid escape: %s\n", yytext); }
<STRING>\n		{ lex_panic("newline in string\n"); }

<INITIAL>{print}	{ return PRINT; }
<INITIAL>{var}		{ return VAR; }
<INITIAL>"="		{ return EQUALS; }

<INITIAL>"!("       { BEGIN MACRO; strbuf_clear(strbuf); }
<MACRO>[^\\)]*     { strbuf_append(strbuf, yytext); }
<MACRO>"\\)"          { strbuf_append_c(strbuf, yytext[1]); }
<MACRO>"\\("		    { strbuf_append_c(strbuf, yytext[1]); }
<MACRO>"\\"           { strbuf_append(strbuf, yytext); }
<MACRO>")"            { yylval.str = strdup(strbuf); BEGIN INITIAL; return MACRO_CONTENT; }

<INITIAL>{hex}		{ yylval.number = strtol(yytext + 2, NULL, 16); return NUM; }
<INITIAL>{dec}		{ yylval.number = strtol(yytext, NULL, 10); return NUM; }
<INITIAL>{oct}		{ yylval.number = strtol(yytext + 1, NULL, 8); return NUM; }
<INITIAL>{bin}		{ yylval.number = strtol(yytext + 2, NULL, 2); return NUM; }
<INITIAL>{char}		{ yylval.ch = yytext[1]; return CHAR; }
<INITIAL>{id}		{ yylval.id = strdup(yytext); return ID; }
<INITIAL>{whitespace}+	{ }

<INITIAL>.		    { lex_panic("unknown token: %s\n", yytext); }
