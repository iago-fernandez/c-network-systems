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
#include <errno.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_EVENTS 64
#define BUFFER_SIZE 4096

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
                while (1) {
                    struct sockaddr_in client_addr;
                    socklen_t client_len = sizeof(client_addr);
                    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

                    if (client_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        else {
                            perror("accept failed");
                            break;
                        }
                    }

                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                    printf("Accepted connection from %s:%d (fd: %d).\n", client_ip, ntohs(client_addr.sin_port), client_fd);

                    set_non_blocking(client_fd);

                    event.data.fd = client_fd;
                    event.events = EPOLLIN | EPOLLET;

                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                        perror("epoll_ctl EPOLL_CTL_ADD client failed");
                        close(client_fd);
                    }
                }
            }
            else {
                int client_fd = events[i].data.fd;
                while (1) {
                    char buffer[BUFFER_SIZE];
                    ssize_t bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

                    if (bytes_read > 0) {
                        printf("Received %zd bytes from client fd %d.\n", bytes_read, client_fd);
                    }
                    else if (bytes_read == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        perror("recv failed");
                        close(client_fd);
                        break;
                    }
                    else if (bytes_read == 0) {
                        printf("Client fd %d disconnected.\n", client_fd);
                        close(client_fd);
                        break;
                    }
                }
            }
        }
    }

    close(server_fd);
    close(epoll_fd);
}