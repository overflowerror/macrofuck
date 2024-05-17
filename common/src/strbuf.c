#include <string.h>

#include "strbuf.h"

strbuf_t strbuf_new(void) {
    strbuf_t buffer = list_new(char);
    list_add(buffer, '\0');
    return buffer;
}

strbuf_t strbuf_from(const char* str) {
    strbuf_t buffer = strbuf_new();
    strbuf_append(buffer, str);
    return buffer;
}


strbuf_t _strbuf_replace(strbuf_t buffer, char* needle, char* replace) {
    size_t offset = 0;

    size_t needle_length = strlen(needle);
    size_t replace_length = strlen(replace);

    char* result;
    while((result = strstr(buffer + offset, needle)) != NULL) {
        offset = result - buffer;
        if (replace_length == needle_length) {
            memcpy(result, replace, replace_length);
        } else if (replace_length > needle_length) {
            list_ensure_space(buffer, 1, replace_length - needle_length);
            memmove(result + replace_length, result + needle_length, strlen(buffer) - offset - needle_length + 1);
            memcpy(result, replace, replace_length);
        } else {
            memcpy(result, replace, replace_length);
            memmove(result + replace_length, result + needle_length, strlen(buffer) - offset - needle_length + 1);
        }
    }

    return buffer;
}
