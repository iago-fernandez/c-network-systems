/**
 * @file epoll_server.c
 * @brief Implementation of the asynchronous epoll event loop.
 */
#include "server/epoll_server.h"
#include "common/net_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>

#define MAX_EVENTS 64

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

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        die_with_error("epoll_create1 failed");
    }

    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];

    memset(&event, 0, sizeof(struct epoll_event));
    event.data.fd = server_fd;
    event.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        die_with_error("epoll_ctl EPOLL_CTL_ADD failed");
    }

    printf("Server listening on port %s...\n", port);

    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            die_with_error("epoll_wait failed");
        }

        for (int i = 0; i < num_events; i++) {
            if (events[i].data.fd == server_fd) {
                printf("New connection event detected on server socket.\n");
                /* Connection acceptance logic will be implemented next */
            }
            else {
                printf("Data event detected on client socket.\n");
                /* Data read/write logic will be implemented next */
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
}