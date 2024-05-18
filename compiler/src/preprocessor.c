#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <alloca.h>
#include <libgen.h>

#include "y.tab.h"

#include <error.h>
#include <list.h>
#include <dict.h>
#include <alloc.h>
#include <strbuf.h>

void push_file_to_yy_stack(FILE*);
void push_str_to_yy_stack(const char*);
int yylex(void);

extern char* yytext;
extern YYSTYPE yylval;

extern const char* input_filename;

static void handle_include(void) {
    int token = yylex();

    if (token != STR) {
        fprintf(stderr, "preprocessor: include: file name expected, got: %s\n", yytext);
        panic("preprocessor syntax error");
    }

    char* input = strdup(input_filename);
    char* relative_path = strdup(dirname(input));
    free(input);

    char* filename = strdup(yylval.str);

    if (filename[0] != '/') {
        strbuf_t buffer = strbuf_from(relative_path);
        strbuf_append_c(buffer, '/');
        strbuf_append(buffer, filename);
        free(filename);
        filename = strdup(buffer);
        strbuf_free(buffer);
    }

    free(relative_path);

    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        panic("file not found");
    }

    free(filename);

    push_file_to_yy_stack(file);
}

struct macro {
    char* name;
    char** argument_list;
    char* content;
};

static dict_t* macros = NULL;

static void handle_macro_definition(void) {
    struct macro* macro = safe_malloc(sizeof(struct macro));
    macro->argument_list = list_new(char*);

    int token = yylex();
    if (token != ID) {
        fprintf(stderr, "preprocessor: macro: identifier expected, got: %s\n", yytext);
        panic("preprocessor syntax error");
    }
    macro->name = yylval.id;

    token = yylex();
    if (token != OPENING_BRACKETS) {
        fprintf(stderr, "preprocessor: macro: ( expected, got: %s\n", yytext);
        panic("preprocessor syntax error");
    }

    bool is_first = true;

    while (true) {
        token = yylex();

        if (token == CLOSING_BRACKETS) {
            break;
        }

        if (!is_first) {
            if (token != COMMA) {
                fprintf(stderr, "preprocessor: macro: , expected, got: %s\n", yytext);
                panic("preprocessor syntax error");
            }
        } else {
            if (token != ID) {
                fprintf(stderr, "preprocessor: macro: identifier expected, got: %s\n", yytext);
                panic("preprocessor syntax error");
            }
            list_add(macro->argument_list, yylval.id);
        }
    }

    token = yylex();
    if (token != MACRO_CONTENT) {
        fprintf(stderr, "preprocessor: macro: body expected, got: %s\n", yytext);
        panic("preprocessor syntax error");
    }

    macro->content = yylval.str;

    if (macros == NULL) {
        macros = dict_new();
    }

    printf("new macro: %s\n  ", macro->name);
    for (size_t i = 0; i < list_size(macro->argument_list); i++) {
        printf("%s", macro->argument_list[i]);
        if (i == list_size(macro->argument_list) - 1) {
            printf(", ");
        }
    }
    printf("\n  {%s}\n", macro->content);

    dict_put(macros, macro->name, macro);
}

char** split_arguments(char* str) {
    strbuf_t* argument_list = list_new(char*);

    strbuf_t argument = strbuf_new();

    while(*str) {
        if (*str != ',') {
            strbuf_append_c(argument, *str);
        } else {
            list_add(argument_list, strdup(argument));
            strbuf_clear(argument);
        }
        str++;
    }

    if (strlen(argument) > 0) {
        list_add(argument_list, strdup(argument));
    }

    strbuf_free(argument);

    return argument_list;
}

void handle_macro_call(void) {
    char* id = yylval.id;
    id[strlen(id) - 2] = '\0';

    int token = yylex();
    if (token != MACRO_CONTENT) {
        fprintf(stderr, "preprocessor: macro: arguments expected, got: %s\n", yytext);
        panic("preprocessor syntax error");
    }

    char* args_string = yylval.str;

    printf("call to %s: %s\n", id, args_string);

    struct macro* macro = dict_get(macros, id);
    if (macro == NULL) {
        fprintf(stderr, "preprocessor: macro: not found: %s\n", id);
        panic("preprocessor error");
    }

    strbuf_t* args = split_arguments(args_string);

    size_t given_argc = list_size(args);
    size_t expected_argc = list_size(macro->argument_list);

    if (given_argc != expected_argc) {
        fprintf(stderr, "preprocessor: macro: %s: argument count mismatch: %zu/%zu\n", id, given_argc, expected_argc);
        panic("preprocessor error");
    }

    strbuf_t result = strbuf_from(macro->content);

    for (size_t i = 0; i < expected_argc; i++) {
        char* arg_id_with_prefix = alloca(strlen(macro->argument_list[i]) + 1 + 1);
        arg_id_with_prefix[0] = '$';
        arg_id_with_prefix[1] = '\0';
        strcat(arg_id_with_prefix, macro->argument_list[i]);

        printf("arg: %s -> %s\n", arg_id_with_prefix, args[i]);

        strbuf_replace(result, arg_id_with_prefix, args[i]);

        strbuf_free(args[i]);
    }

    list_free(args);

    printf("result: %s\n", result);

    push_str_to_yy_stack(result);

    //strbuf_free(result);
}

extern int preproc_lex(void) {
    int token = yylex();

    //printf("pre: %d\n", token);

    bool is_normal_token = false;

    while (!is_normal_token) {
        switch (token) {
            case PRE_INCLUDE:
                handle_include();
                token = yylex();
                break;
            case PRE_MACRO:
                handle_macro_definition();
                token = yylex();
                break;
            case MACRO_CALL:
                handle_macro_call();
                token = yylex();
                break;
            default:
                is_normal_token = true;
        }
    }

    //printf("post: %d\n", token);

    return token;
}

