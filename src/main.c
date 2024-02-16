#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

#include "error.h"
#include "ast.h"
#include "codegen.h"
#include "plugins.h"

#define EXIT_ARGS_ERROR (3)

extern FILE* yyin;
extern int yyparse(void);
extern int yydebug;

#define DEBUG_MODE (0)

struct program* program;

void help(void) {
	printf("// TODO\n");
}

int main(int argc, char** argv) {
    if (DEBUG_MODE) yydebug = 1;

	FILE* input;
	FILE* output = NULL;

    add_plugin(NULL);

	int opt;
	while((opt = getopt(argc, argv, "o:p:")) != -1) {
		switch(opt) {
            case 'p':
                add_plugin(optarg);
                break;
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

    if (DEBUG_MODE) fprintf(stderr, INFO("parsing\n"));

	yyin = input;
	int result = yyparse();
	if (result != 0) {
		return result;
	}


    if (DEBUG_MODE) fprintf(stderr, INFO("loading modules\n"));
    load_plugins();

    if (DEBUG_MODE) fprintf(stderr, INFO("generating code\n"));
	result = codegen(output, program);
	if (result != 0) {
		return result;
	}

	return 0;
}
