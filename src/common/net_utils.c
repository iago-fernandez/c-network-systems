/**
 * @file net_utils.c
 * @brief Implementation of common network utilities.
 */
#include "common/net_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

void die_with_error(const char* error_message) {
    perror(error_message);
    exit(EXIT_FAILURE);
}

int setup_tcp_server_socket(const char* service) {
    struct addrinfo hints;
    struct addrinfo* servinfo;
    struct addrinfo* p;
    int serv_sock;
    int yes = 1;
    int rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, service, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(EXIT_FAILURE);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((serv_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        if (setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            die_with_error("setsockopt");
        }

        if (bind(serv_sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(serv_sock);
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        fprintf(stderr, "failed to bind\n");
        exit(EXIT_FAILURE);
    }

    if (listen(serv_sock, 10) == -1) {
        die_with_error("listen");
    }

    return serv_sock;
}