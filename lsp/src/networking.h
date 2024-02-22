#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdio.h>

typedef struct {
    FILE* request_body;
    FILE* response_body;
} request_t;

typedef void (*handler_t) (request_t*);

int run_server(const char*, handler_t);

#endif
