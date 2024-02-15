#ifndef AST_H
#define AST_H

#include <stdlib.h>

enum literal_kind {
	NUMBER_LITERAL,
	CHAR_LITERAL,
	STRING_LITERAL,
};

enum expression_kind {
	LITERAL,
	VARIABLE,
    MACRO,
};

enum expression_type {
	INTEGER,
	CHARACTER,
	STRING,
	UNKNOWN_TYPE,
};

enum statement_kind {
	PRINT_STATEMENT,
	DECL_STATEMENT,
    MACRO_STATEMENT,
};

struct literal_expression {
	enum literal_kind kind;
	union {
		long long number;
		char ch;
		char* str;	
	};
};

struct variable_expression {
	char* id;
};

struct macro_expression {
    char* id;
    char* argument;
};

struct expression {
	enum expression_kind kind;
	enum expression_type type;
	union {
		struct literal_expression literal;
		struct variable_expression variable;
        struct macro_expression macro;
	};
};

struct print_statement {
	struct expression* value;
};

struct declaration_statement {
	char* id;
	struct expression* value;
};

struct macro_statement {
    struct expression* expr;
};

struct statement {
	enum statement_kind kind;
	union {
		struct print_statement print;
		struct declaration_statement decl;
        struct macro_statement macro;
	};
};

struct program {
	size_t length;
	struct statement** statements;
};

struct program* program_new(void);
void program_add_statement(struct program*, struct statement*);

struct statement* print_statement_new(struct expression*);
struct statement* declaration_statement_new(char*, struct expression*);
struct statement* macro_expression(struct expression*);

struct expression* literal_expression_char_new(char);
struct expression* literal_expression_str_new(char*);
struct expression* literal_expression_num_new(long long);
struct expression* variable_expression_new(char*);
struct expression* macro_expression_new(char*, char*);

#endif 
