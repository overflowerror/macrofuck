#ifndef STRBUF_H
#define STRBUF_H

#include <string.h>

#include "list.h"

typedef char* strbuf_t;

#define strbuf_clear(b) { \
	if (b == NULL) { \
		b = strbuf_new(); \
	} else { \
		list_header(b)->length = 1; \
		b[0] = '\0'; \
	} \
}
#define strbuf_append(b, s) { \
	size_t l = strlen(s); \
	(b) = list_ensure_space(b, 1, l); \
	strcat(b, s); \
	list_header(b)->length += l; \
}

#define strbuf_append_c(b, c) { \
    (b) = list_ensure_space(b, 1, 1); \
	b[list_size(b) - 1] = c; \
    b[list_size(b)] = '\0'; \
    list_header(b)->length += 1; \
}

#define strbuf_replace(b, n, r) { \
    (b) = _strbuf_replace(b, n, r); \
}

strbuf_t strbuf_new(void);
strbuf_t _strbuf_replace(strbuf_t, char*, char*);

#define strbuf_free(b) list_free(b)

#endif
