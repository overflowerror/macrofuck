#ifndef NETWORKING_H
#define NETWORKING_H

#include <stdio.h>

#include "header.h"

typedef struct {
    header_t* request_header;
    FILE* request_body;
    FILE* response_body;
} request_t;

typedef void (*handler_t) (request_t*);

int run_server(const char*, handler_t);

#endif
