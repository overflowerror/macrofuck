#include <stdio.h>
#include <alloca.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <alloc.h>
#include <dict.h>

#include "header.h"

header_t* header_new(void) {
    header_t* header = safe_malloc(sizeof (header_t));
    header->method = NULL;
    header->uri = NULL;
    header->headers = dict_new();

    return header;
}

header_t* header_parse(FILE* conn) {
    fprintf(stderr, "parsing headers\n");
    header_t* header = header_new();

    #define HEADER_BUFFER_LENGTH (1<<10)
    char* buffer = alloca(HEADER_BUFFER_LENGTH);
    size_t buffer_index = 0;

    char* key = NULL;
    char* value = NULL;

    bool is_first_line = true;
    bool last_line_was_empty = false;
    while (true) {
        int c = fgetc(conn);
        fprintf(stderr, "c: %c, %d, %d, %s, %s, %s\n", c, is_first_line, last_line_was_empty, buffer, key, value);
        if (c == EOF) {
            break;
        }
        if (c == '\r') {
            c = fgetc(conn);
            if (c == '\n') {
                if (last_line_was_empty) {
                    break;
                } else {
                    if (is_first_line) {
                        char* method = strtok(buffer, " ");
                        char* uri = strtok(NULL, " ");
                        char* proto = strtok(NULL, " ");

                        // defer fail to when fields are read
                        if (method) {
                            header->method = strdup(method);
                        }
                        if (uri) {
                            header->uri = strdup(uri);
                        }
                        if (proto) {
                            header->proto = strdup(proto);
                        }

                        is_first_line = false;
                    } else {
                        if (!key) {
                            fprintf(stderr, "no key for value - ignoring\n");
                        } else {
                            value = strdup(buffer);
                            if (!value) {
                                fprintf(stderr, "unable to dup header value: %s\n", strerror(errno));
                                return NULL;
                            }
                            header_add(header, key, value);
                        }
                    }
                }
            } else {
                fprintf(stderr, "header parse error, unexpected %d at header %s\n", c, key);
            }

            *buffer = '\0';
            buffer_index = 0;
            key = NULL;
            value = NULL;
            last_line_was_empty = true;
        } else if (key == NULL && c == ':' && !is_first_line) {
            c = fgetc(conn);
            if (c == ' ') {
                key = strdup(buffer);
                if (!key) {
                    fprintf(stderr, "unable to dup header key: %s\n", strerror(errno));
                    return NULL;
                }
                *buffer = '\0';
                buffer_index = 0;
            }
        } else {
            last_line_was_empty = false;
            if (buffer_index + 1 >= HEADER_BUFFER_LENGTH) {
                fprintf(stderr, "header too long - truncating\n");
            } else {
                buffer[buffer_index] = c;
                buffer[++buffer_index] = '\0';
            }
        }
    }

    return header;
}

void header_add(header_t* header, const char* key, const char* value) {
    dict_put(header->headers, key, (void*) value);
}

const char* header_get(header_t* header, const char* key) {
    return dict_get(header->headers, key);
}
