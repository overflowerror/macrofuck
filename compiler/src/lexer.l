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
rmap    "rmap"
in      "in"

char    "'"([^\\]|[\\]n|[\\]t|[\\]{2})"'"
id		[a-zA-Z_][a-zA-Z0-9_]*

include "#include"
macro   "#macro"

%option noyywrap
%option nodefault
%option nounput
%option noinput
%option yylineno

%x COMMENT
%x STRING
%x MACRO_DEF
%x MACRO_VAL
%x MACRO_ARGS

%{

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <strbuf.h>
#include <error.h>

#include "y.tab.h"

extern void push_file_to_yy_stack(FILE* file) {
    YY_BUFFER_STATE state = yy_create_buffer(file, YY_BUF_SIZE);
    yypush_buffer_state(state);
}

extern void push_str_to_yy_stack(const char* str) {
    yypush_buffer_state(YY_CURRENT_BUFFER);
    yy_scan_string(str);
}

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

strbuf_t strbuf = NULL;
int nested_braces = 0;

%}

%%

%{

%}

<INITIAL>"/*"	       { BEGIN(COMMENT); }
<COMMENT>"*/"	       { BEGIN(INITIAL); }
<COMMENT>[^*\n]	       { }
<COMMENT>\n	           { }
<COMMENT>"*"[^*\/\n]*  { }

<INITIAL>"//"[^\n]*    { /* single line comment */ }

<INITIAL>{include}  { return PRE_INCLUDE; }
<INITIAL>{macro}    { BEGIN MACRO_DEF; return PRE_MACRO; }
<MACRO_DEF>{id}     { yylval.id = strdup(yytext); return ID; }
<MACRO_DEF>"("      { return OPENING_BRACKETS; }
<MACRO_DEF>")"      { return CLOSING_BRACKETS; }
<MACRO_DEF>","      { return COMMA; }
<MACRO_DEF>{whitespace}+	{ }
<MACRO_DEF>"{"      { BEGIN MACRO_VAL; nested_braces = 0; strbuf_clear(strbuf); }
<MACRO_DEF>.        { lex_panic("unknown token: %s\n", yytext); }
<MACRO_VAL>"{"      { nested_braces++; strbuf_append_c(strbuf, yytext[0]); }
<MACRO_VAL>"}"      {
                        if (nested_braces-- == 0) {
                            BEGIN INITIAL;
                            yylval.str = strdup(strbuf);
                            return MACRO_CONTENT;
                        } else {
                            strbuf_append_c(strbuf, yytext[0]);
                        }
                    }
<MACRO_VAL>{whitespace}+ { strbuf_append(strbuf, yytext); }
<MACRO_VAL>.        { strbuf_append_c(strbuf, yytext[0]); }

<INITIAL>{id}"$("   { strbuf_clear(strbuf); yylval.id = strdup(yytext); BEGIN MACRO_ARGS; return MACRO_CALL; }
<MACRO_ARGS>[^\\)]* { strbuf_append(strbuf, yytext); }
<MACRO_ARGS>"\\)"   { strbuf_append_c(strbuf, yytext[1]); }
<MACRO_ARGS>"\\("	{ strbuf_append_c(strbuf, yytext[1]); }
<MACRO_ARGS>"\\"    { strbuf_append(strbuf, yytext); }
<MACRO_ARGS>")"     { yylval.str = strdup(strbuf); BEGIN INITIAL; return MACRO_CONTENT; }

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
<INITIAL>{rmap}     { return RMAP; }
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

<INITIAL>{hex}		{ yylval.number = strtol(yytext + 2, NULL, 16); return NUM; }
<INITIAL>{dec}		{ yylval.number = strtol(yytext, NULL, 10); return NUM; }
<INITIAL>{oct}		{ yylval.number = strtol(yytext + 1, NULL, 8); return NUM; }
<INITIAL>{bin}		{ yylval.number = strtol(yytext + 2, NULL, 2); return NUM; }
<INITIAL>{char}		{ yylval.ch = char_escape(yytext); return CHAR; }
<INITIAL>{id}		{ yylval.id = strdup(yytext); return ID; }
<INITIAL>{whitespace}+	{ }

<<EOF>>             {
                        yypop_buffer_state();
                        if (!YY_CURRENT_BUFFER) {
                            yyterminate();
                        }
                    }

<INITIAL>.		    { lex_panic("unknown token: %s\n", yytext); }
