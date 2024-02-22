#include <stdio.h>

#include <dlfcn.h>

#include "plugins.h"
#include "error.h"

struct plugin {
    const char* name;
    void* handle;
}* plugins = NULL;

void add_plugin(const char* name) {
    if (!plugins) {
        plugins = list_new(struct plugin);
    }

    struct plugin new_plugin = {
            .name = name,
            .handle = NULL,
    };

    list_add(plugins, new_plugin);
}

void load_plugins(void) {
    if (!plugins) {
        plugins = list_new(struct plugin);
    }

    size_t size = list_size(plugins);
    for (size_t i = 0; i < size; i++) {
        const char* name = plugins[i].name;
        void* handle = dlopen(name, RTLD_NOW | RTLD_LOCAL);
        if (!handle) {
            fprintf(stderr, "plugin %s: %s\n", name, dlerror());
            panic("failed to load plugin");
        }
        plugins[i].handle = handle;
    }
}

macro_t find_macro(const char* name) {
    size_t size = list_size(plugins);
    for (size_t i = 0; i < size; i++) {
        void* candidate = dlsym(plugins[i].handle, name);
        if (candidate) {
            return (macro_t) candidate;
        }
    }
    fprintf(stderr, "No plugin for macro found: %s\n", name);
    panic("macro not found");
    return NULL;
}
