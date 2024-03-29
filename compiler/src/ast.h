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
    CALCULATION,
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
    ASSIGNMENT_STATEMENT,
    MACRO_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
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

enum calc_operator {
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    MODULO,
    EQUALS,
    NOT_EQUALS,
    CONJUNCTION,
    DISJUNCTION,
    NEGATION,
};

struct calc_expression {
    struct expression* operand1;
    struct expression* operand2;
    enum calc_operator operator;
};

struct expression {
	enum expression_kind kind;
	enum expression_type type;
	union {
		struct literal_expression literal;
		struct variable_expression variable;
        struct macro_expression macro;
        struct calc_expression calc;
	};
};

struct print_statement {
	struct expression* value;
};

struct assignment_statement {
    char* id;
    struct expression* value;
};

struct macro_statement {
    struct expression* expr;
};

struct if_statement {
    struct expression* condition;
    struct block* if_block;
    struct block* else_block;
};

struct while_statement {
    struct expression* condition;
    struct block* block;
};

struct statement {
	enum statement_kind kind;
	union {
		struct print_statement print;
		struct assignment_statement assignment;
        struct macro_statement macro;
        struct if_statement if_else;
        struct while_statement while_loop;
	};
};

struct block {
	size_t length;
	struct statement** statements;
};

struct block* block_new(void);
void block_add_statement(struct block*, struct statement*);

struct statement* print_statement_new(struct expression*);
struct statement* declaration_statement_new(struct statement*);
struct statement* assignment_statement_new(char*, struct expression*);
struct statement* macro_statement_new(struct expression*);
struct statement* if_statement_new(struct expression*, struct block*, struct block*);
struct statement* while_statement_new(struct expression*, struct block*);

struct expression* literal_expression_char_new(char);
struct expression* literal_expression_str_new(char*);
struct expression* literal_expression_num_new(long long);
struct expression* variable_expression_new(char*);
struct expression* macro_expression_new(char*, char*);
struct expression* calc_expression_new(struct expression*, struct expression*, enum calc_operator);

#endif 
