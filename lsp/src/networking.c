#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <alloca.h>

#include <sys/socket.h>
#include <netdb.h>

#include "networking.h"

static char* split_port(char* addr) {
    char* port_part = addr;
    while(*port_part != '\0') {
        if (*port_part == ':') {
            *port_part = '\0';
            return port_part + 1;
        }
        port_part++;
    }

    return NULL;
}

static int prepare_socket(const char* bind_addr) {
    char* host_part = strdup(bind_addr);
    if (!host_part) {
        fprintf(stderr, "error duping bind addr: %s\n", strerror(errno));
        return -1;
    }

    char* port_part = split_port(host_part);
    if (!port_part) {
        return -1;
    }

    printf("  addr %s port %s\n", host_part, port_part);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* result;
    struct addrinfo* result_candidates;
    int error;
    if ((error = getaddrinfo(host_part, port_part, &hints, &result_candidates)) != 0) {
        fprintf(stderr, "error parsing bind addr: %s\n", gai_strerror(error));
        return -1;
    }

    for (result = result_candidates; result; result = result->ai_next) {
        int sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if (sock < 0) {
            fprintf(stderr, "warn: socket: candidate failed: %s\n", strerror(errno));
            continue;
        }

        printf("binding to %s\n", bind_addr);
        if (bind(sock, result->ai_addr, result->ai_addrlen) < 0) {
            close(sock);
            fprintf(stderr, "warn: bind: candidate failed: %s\n", strerror(errno));
            continue;
        }

        printf("listening\n");
        if (listen(sock, 16) < 0) {
            close(sock);
            fprintf(stderr, "warn: bind: candidate failed: %s\n", strerror(errno));
            continue;
        }

        freeaddrinfo(result_candidates);
        free(host_part);

        return sock;
    }

    freeaddrinfo(result_candidates);
    free(host_part);

    fprintf(stderr, "error: no suitable candidate found\n");
    return -1;
}

static request_t prepare_request_obj(int fd) {
    request_t request;
    memset(&request, 0, sizeof(request_t));

    int fd2 = dup(fd);
    if (fd2 < 0) {
        fprintf(stderr, "error while duping confd: %s\n", strerror(errno));
        return request;
    }

    request.request_body = fdopen(fd, "r");
    request.response_body = fdopen(fd2, "w");

    request.request_header = header_parse(request.request_body);

    return request;
}

static void handle_connection(int fd, handler_t handler) {
    int pid = fork();
    if (pid < 0) {
        fprintf(stderr, "connection: error: fork failed: %s\n", strerror(errno));
    } else if (pid == 0) {
        request_t request = prepare_request_obj(fd);

        printf("starting handler\n");
        handler(&request);
        printf("handler returned\n");

        if (request.request_body) {
            fclose(request.request_body);
        }
        if (request.response_body) {
            fclose(request.response_body);
        }

        printf("connection closed\n");
    } else {
        // we are the parent
        close(fd);
        return;
    }
}

int run_server(const char* bind_addr, handler_t handler) {
    int sock = prepare_socket(bind_addr);
    if (sock < 0) {
        return -1;
    }

    while (true) {
        // we don't care for the remote address right now
        int fd = accept(sock, NULL, 0);
        fprintf(stderr, "new connection: %d\n", fd);
        if (fd < 0) {
            fprintf(stderr, "warn: accept failed: %s\n", strerror(errno));

        }

        handle_connection(fd, handler);
    }
}
