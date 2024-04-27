#include <string.h>

#include "builtins.h"

#define decl(n) region_t* n(FILE*, scope_t*, size_t, region_t**)

decl(print);
decl(read_char);
decl(read);

decl(to_str);

static struct builtin_list_item {
    const char* name;
    builtin_t function;
} builtins[] = {
    {"print", print},
    {"read_char", read_char},
    {"read", read},

    {"to_str", to_str},
};

builtin_t find_builtin(const char* name) {
    for (size_t i = 0; i < sizeof(builtins) / sizeof(struct builtin_list_item); i++) {
        if (strcmp(name, builtins[i].name) == 0) {
            return builtins[i].function;
        }
    }

    return NULL;
}
