#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <stdbool.h>

enum literal_kind {
	NUMBER_LITERAL,
	CHAR_LITERAL,
	STRING_LITERAL,
    ARRAY_LITERAL,
};

enum expression_kind {
	LITERAL,
	VARIABLE,
    MACRO,
    BUILTIN_CALL,
    CALCULATION,
};

enum expression_type {
	INTEGER,
	CHARACTER,
	STRING,
	UNKNOWN_TYPE,
};

enum statement_kind {
	DECL_STATEMENT,
    ASSIGNMENT_STATEMENT,
    EXPR_STATEMENT,
    IF_STATEMENT,
    WHILE_STATEMENT,
    MAP_STATEMENT,
};

struct array_literal {
    size_t length;
    struct expression** values;
};

struct literal_expression {
	enum literal_kind kind;
	union {
		long long number;
		char ch;
		char* str;
        struct array_literal array;
	};
};

struct variable_expression {
	char* id;
    bool is_offset;
    size_t offset;
};

struct macro_expression {
    char* id;
    char* argument;
};

struct builtin_call_expression {
    char* id;
    size_t argument_number;
    struct expression** arguments;
};

enum calc_operator {
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    MODULO,
    EQUALS,
    NOT_EQUALS,
    GREATER_THAN,
    LESS_THAN,
    GREATER_EQUALS,
    LESS_EQUALS,
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
        struct builtin_call_expression builtin_call;
        struct calc_expression calc;
	};
};

struct assignment_statement {
    struct expression* variable;
    struct expression* value;
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

struct map_statement {
    char* index_id;
    char* ref_id;
    char* list_id;
    struct block* block;
};

struct expr_statement {
    struct expression* expr;
};

struct statement {
	enum statement_kind kind;
	union {
		struct assignment_statement assignment;
        struct expr_statement expr;
        struct if_statement if_else;
        struct while_statement while_loop;
        struct map_statement map_loop;
	};
};

struct block {
	size_t length;
	struct statement** statements;
};

struct block* block_new(void);
void block_add_statement(struct block*, struct statement*);

struct statement* declaration_statement_new(struct statement*);
struct statement* assignment_statement_new(struct expression*, struct expression*);
struct statement* expr_statement_new(struct expression*);
struct statement* if_statement_new(struct expression*, struct block*, struct block*);
struct statement* while_statement_new(struct expression*, struct block*);
struct statement* map_statement_new(char*, char*, char*, struct block*);

struct expression* literal_expression_char_new(char);
struct expression* literal_expression_str_new(char*);
struct expression* literal_expression_num_new(long long);
struct expression* literal_expression_array_new(size_t, struct expression**);
struct expression* variable_expression_new(char*);
struct expression* variable_expression_new_offset(char*, size_t);
struct expression* macro_expression_new(char*, char*);
struct expression* builtin_call_expression_new(void);
void builtin_call_expression_add_argument(struct expression*, struct expression*);
struct expression* calc_expression_new(struct expression*, struct expression*, enum calc_operator);

#endif 
