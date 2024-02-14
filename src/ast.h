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
};

enum expression_type {
	INTEGER,
	CHARACTER,
	STRING,
};

enum statement_kind {
	PRINT_STATEMENT,
};

struct literal_expression {
	enum literal_kind kind;
	union {
		long long number;
		char ch;
		char* str;	
	};
};

struct expression {
	enum expression_kind kind;
	enum expression_type type;
	union {
		struct literal_expression literal;
	};
};

struct print_statement {
	struct expression* value;
};

struct statement {
	enum statement_kind kind;
	union {
		struct print_statement print;
	};
};

struct program {
	size_t length;
	struct statement** statements;
};

struct program* program_new(void);
void program_add_statement(struct program*, struct statement*);

struct statement* print_statement_new(struct expression*);

struct expression* literal_expression_char_new(char);
struct expression* literal_expression_str_new(char*);
struct expression* literal_expression_num_new(long long);

#endif 
