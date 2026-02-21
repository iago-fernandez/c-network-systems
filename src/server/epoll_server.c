/**
 * @file epoll_server.c
 * @brief Implementation of the asynchronous epoll event loop.
 */
#include "server/epoll_server.h"
#include "common/net_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

 /**
  * @brief Configures a file descriptor to operate in non-blocking mode.
  *
  * @param fd The file descriptor to modify.
  */
static void set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        die_with_error("fcntl F_GETFL failed");
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        die_with_error("fcntl F_SETFL failed");
    }
}

void start_epoll_server(const char* port) {
    int server_fd = setup_tcp_server_socket(port);
    set_non_blocking(server_fd);

    /* Epoll instance creation and event loop will be implemented later */
}