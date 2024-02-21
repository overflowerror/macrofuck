#ifndef STRBUF_H
#define STRBUF_H

#include <string.h>

#include "list.h"

typedef char* strbuf_t;

#define strbuf_new() list_new(char)
#define strbuf_clear(b) { \
	if (b == NULL) { \
		b = strbuf_new(); \
        list_add(b, '\0'); \
	} else { \
		list_header(b)->length = 1; \
		b[0] = '\0'; \
	} \
}
#define strbuf_append(b, s) { \
	size_t l = strlen(s); \
	list_ensure_space(b, 1, l); \
	strcat(b, s); \
	list_header(b)->length += l; \
}

#define strbuf_append_c(b, c) { \
    list_ensure_space(b, 1, 1); \
	b[list_size(b) - 1] = c; \
    b[list_size(b)] = '\0'; \
    list_header(b)->length += 1; \
}

#endif
