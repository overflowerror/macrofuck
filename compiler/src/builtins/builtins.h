#ifndef BUILTINS_H
#define BUILTINS_H

#include <stdio.h>

#include "../band.h"
#include "../scope.h"

typedef region_t* (*builtin_t)(FILE*, scope_t*, size_t, region_t**);

builtin_t find_builtin(const char*);

#endif
