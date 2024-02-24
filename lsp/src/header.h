#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>

#include <dict.h>

typedef struct {
    char* method;
    char* uri;
    char* proto;
    dict_t* headers;
} header_t;

header_t* header_new(void);
header_t* header_parse(FILE*);
void header_add(header_t*, const char*, const char*);
const char* header_get(header_t*, const char*);

#endif
