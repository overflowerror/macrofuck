#include <stdio.h>

#include "networking.h"

void hello_handler(request_t* request) {
    fprintf(request->response_body, "HTTP/1.0 200 OK\r\n");
    fprintf(request->response_body, "\r\n");
    fprintf(request->response_body, "Hello, World!\n");
    fprintf(request->response_body, "Method: %s\n", request->request_header->method);
    fprintf(request->response_body, "URI: %s\n", request->request_header->uri);
    fprintf(request->response_body, "Host: %s\n", header_get(request->request_header, "Host"));
    fprintf(request->response_body, "User-Agent: %s\n", header_get(request->request_header, "User-Agent"));
}

int main(void) {
    run_server("127.0.0.1:1337", hello_handler);

    return 0;
}
