#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "error.h"
#include "ast.h"

#define EXIT_ARGS_ERROR (3)

extern FILE* yyin;
extern int yyparse(void);

struct program* program;

void help(void) {
	printf("// TODO\n");
}

int main(int argc, char** argv) {
	FILE* input;
	FILE* output = NULL;

	int opt;
	while((opt = getopt(argc, argv, "o:")) != -1) {
		switch(opt) {
			case 'o':
				output = fopen(optarg, "w+");
				if (!output) {
					panic("fopen");
				}
				break;
			default:
				fprintf(stderr, "Unknown option.\n");
				help();
				return EXIT_ARGS_ERROR;
		}
	}

	if (optind >= argc) {
		fprintf(stderr, "No input file given.\n");
		return EXIT_ARGS_ERROR;
	}
	
	input = fopen(argv[optind], "r");
	if (!input) {
		panic("fopen");
	}

	if (!output) {
		output = fopen("m.gen.bf", "w+");
		if (!output) {
			panic("fopen");
		}
	}

	yyin = input;
	int result = yyparse();
	if (result != 0) {
		return result;
	}

	return 0;
}
