#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "error.h"

void _panic(const char* f, const char* s) {
	fprintf(stderr, "panic: %s: %s: %s\n", f, s, strerror(errno));
	exit(3);
}


