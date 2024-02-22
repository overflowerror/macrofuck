#ifndef PLUGINS_H
#define PLUGINS_H

#include <stdio.h>

#include "scope.h"

typedef region_t* (*macro_t)(FILE*, scope_t*, const char*);

void add_plugin(const char*);
void load_plugins(void);
macro_t find_macro(const char*);

#endif
