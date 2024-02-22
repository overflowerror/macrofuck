#include <stdio.h>

#include "networking.h"

void hello_handler(request_t* request) {
    fprintf(request->response_body, "HTTP/1.0 200 OK\r\n");
    fprintf(request->response_body, "\r\n");
    fprintf(request->response_body, "Hello, World!\n");
}

int main(void) {
    run_server("127.0.0.1:1337", hello_handler);

    return 0;
}
