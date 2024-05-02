whitespace	[\n\t\n ]

dec		([1-9][0-9]*)|"0"
oct		"0"[0-9]+
hex		"0x"[0-9a-fA-F]+
bin		"0b"[01]+

var		"var"
if      "if"
else    "else"
while   "while"
map     "map"
in      "in"

char    "'"([^\\]|[\\]n|[\\]t|[\\]{2})"'"
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

#include <strbuf.h>
#include <error.h>

#include "y.tab.h"

char char_escape(const char* yytext) {
    if (yytext[1] == '\\') {
        if (yytext[2] == '\\')     return '\\';
        else if (yytext[2] == 'n') return '\n';
        else if (yytext[2] == 't') return '\t';
        else lex_panic("unknown escape sequence: %s\n", yytext);
    } else {
        return yytext[1];
    }
}

%}

%%

%{

strbuf_t strbuf = NULL;

%}

<INITIAL>"/*"	       { BEGIN(COMMENT); }
<COMMENT>"*/"	       { BEGIN(INITIAL); }
<COMMENT>[^*\n]	       { }
<COMMENT>\n	           { }
<COMMENT>"*"[^*\/\n]*  { }

<INITIAL>"//"[^\n]\n   { /* single line comment */ }

<INITIAL>";"		{ return SEMICOLON; }

<INITIAL>\"		    { BEGIN STRING; strbuf_clear(strbuf); }
<STRING>[^\\"\n]*   { strbuf_append(strbuf, yytext); }
<STRING>\\n		    { strbuf_append_c(strbuf, '\n'); }
<STRING>\\t		    { strbuf_append_c(strbuf, '\t'); }
<STRING>\\[0-7]*	{ strbuf_append_c(strbuf, strtol(yytext+1, 0, 8)); }
<STRING>\\[\\"]		{ strbuf_append_c(strbuf, yytext[1]); }
<STRING>\"          { yylval.str = strdup(strbuf); BEGIN INITIAL; return STR; }
<STRING>\\.	        { lex_panic("invalid escape: %s\n", yytext); }
<STRING>\n		    { lex_panic("newline in string\n"); }

<INITIAL>{var}		{ return VAR; }
<INITIAL>{if}       { return IF; }
<INITIAL>{else}     { return ELSE; }
<INITIAL>{while}    { return WHILE; }
<INITIAL>{map}      { return MAP; }
<INITIAL>{in}       { return IN; }

<INITIAL>"="		{ return ASSIGNMENT; }

<INITIAL>"+"		{ return PLUS; }
<INITIAL>"-"		{ return MINUS; }
<INITIAL>"*"		{ return TIMES; }
<INITIAL>"/"		{ return DIVIDE; }
<INITIAL>"%"		{ return MOD; }

<INITIAL>"=="       { return EQUAL; }
<INITIAL>"!="       { return NOT_EQUAL; }
<INITIAL>">"        { return GREATER; }
<INITIAL>"<"        { return LESS; }
<INITIAL>">="       { return GREATER_EQUAL; }
<INITIAL>"<="       { return LESS_EQUAL; }
<INITIAL>"&&"       { return AND; }
<INITIAL>"||"       { return OR; }
<INITIAL>"!"        { return NOT; }

<INITIAL>"("        { return OPENING_BRACKETS; }
<INITIAL>")"        { return CLOSING_BRACKETS; }
<INITIAL>"{"        { return OPENING_BRACES; }
<INITIAL>"}"        { return CLOSING_BRACES; }
<INITIAL>"["        { return OPENING_SQ_BRACKETS; }
<INITIAL>"]"        { return CLOSING_SQ_BRACKETS; }
<INITIAL>","        { return COMMA; }

<INITIAL>"$("       { BEGIN MACRO; strbuf_clear(strbuf); }
<MACRO>[^\\)]*      { strbuf_append(strbuf, yytext); }
<MACRO>"\\)"        { strbuf_append_c(strbuf, yytext[1]); }
<MACRO>"\\("		{ strbuf_append_c(strbuf, yytext[1]); }
<MACRO>"\\"         { strbuf_append(strbuf, yytext); }
<MACRO>")"          { yylval.str = strdup(strbuf); BEGIN INITIAL; return MACRO_CONTENT; }

<INITIAL>{hex}		{ yylval.number = strtol(yytext + 2, NULL, 16); return NUM; }
<INITIAL>{dec}		{ yylval.number = strtol(yytext, NULL, 10); return NUM; }
<INITIAL>{oct}		{ yylval.number = strtol(yytext + 1, NULL, 8); return NUM; }
<INITIAL>{bin}		{ yylval.number = strtol(yytext + 2, NULL, 2); return NUM; }
<INITIAL>{char}		{ yylval.ch = char_escape(yytext); return CHAR; }
<INITIAL>{id}		{ yylval.id = strdup(yytext); return ID; }
<INITIAL>{whitespace}+	{ }

<INITIAL>.		    { lex_panic("unknown token: %s\n", yytext); }
