/**
 * @file epoll_server.c
 * @brief Implementation of the asynchronous epoll event loop with context management.
 */
#include "server/epoll_server.h"
#include "server/client_context.h"
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

/**
 * @brief Handles data reading for a connected client using a state machine.
 *
 * @param ctx The client context containing the connection state.
 */
static void handle_client_data(ClientContext* ctx) {
    ssize_t bytes_read;

    while (1) {
        if (ctx->state == STATE_READING_HEADER) {
            size_t remaining = sizeof(ctx->header_buffer) - ctx->header_bytes_read;
            bytes_read = recv(ctx->fd, ctx->header_buffer + ctx->header_bytes_read, remaining, 0);

            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                perror("recv header failed");
                close(ctx->fd);
                free_client_context(ctx);
                free(ctx);
                return;
            }
            else if (bytes_read == 0) {
                printf("Client fd %d disconnected during header read.\n", ctx->fd);
                close(ctx->fd);
                free_client_context(ctx);
                free(ctx);
                return;
            }

            ctx->header_bytes_read += bytes_read;

            if (ctx->header_bytes_read == sizeof(ctx->header_buffer)) {
                ProtocolHeader header;
                deserialize_header(ctx->header_buffer, &header);

                ctx->expected_payload_length = header.payload_length;
                ctx->message_type = header.message_type;

                if (ctx->expected_payload_length > 0) {
                    ctx->payload_buffer = (uint8_t*)malloc(ctx->expected_payload_length);
                    if (!ctx->payload_buffer) {
                        perror("malloc payload failed");
                        close(ctx->fd);
                        free(ctx);
                        return;
                    }
                    ctx->state = STATE_READING_PAYLOAD;
                }
                else {
                    printf("Received header-only message. Type: %d\n", ctx->message_type);
                    reset_client_context(ctx);
                }
            }
        }
        else if (ctx->state == STATE_READING_PAYLOAD) {
            size_t remaining = ctx->expected_payload_length - ctx->payload_bytes_read;
            bytes_read = recv(ctx->fd, ctx->payload_buffer + ctx->payload_bytes_read, remaining, 0);

            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                perror("recv payload failed");
                close(ctx->fd);
                free_client_context(ctx);
                free(ctx);
                return;
            }
            else if (bytes_read == 0) {
                printf("Client fd %d disconnected during payload read.\n", ctx->fd);
                close(ctx->fd);
                free_client_context(ctx);
                free(ctx);
                return;
            }

            ctx->payload_bytes_read += bytes_read;

            if (ctx->payload_bytes_read == ctx->expected_payload_length) {
                printf("Received message. Type: %d, Length: %d\n", ctx->message_type, ctx->expected_payload_length);
                /* Business logic would be invoked here */
                reset_client_context(ctx);
            }
        }
    }
}

void start_epoll_server(const char* port) {
    int server_fd = setup_tcp_server_socket(port);
    set_non_blocking(server_fd);

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        die_with_error("epoll_create1 failed");
    }

    /* We wrap the server socket in a context to unify pointer handling in epoll */
    ClientContext* server_ctx = (ClientContext*)malloc(sizeof(ClientContext));
    init_client_context(server_ctx, server_fd);

    struct epoll_event event;
    struct epoll_event events[MAX_EVENTS];

    memset(&event, 0, sizeof(struct epoll_event));
    event.data.ptr = server_ctx;
    event.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        die_with_error("epoll_ctl EPOLL_CTL_ADD failed");
    }

    printf("Server listening on port %s (Binary Protocol V1)...\n", port);

    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            die_with_error("epoll_wait failed");
        }

        for (int i = 0; i < num_events; i++) {
            ClientContext* ctx = (ClientContext*)events[i].data.ptr;

            if (ctx->fd == server_fd) {
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

                    ClientContext* new_client_ctx = (ClientContext*)malloc(sizeof(ClientContext));
                    init_client_context(new_client_ctx, client_fd);

                    event.data.ptr = new_client_ctx;
                    event.events = EPOLLIN | EPOLLET;

                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                        perror("epoll_ctl EPOLL_CTL_ADD client failed");
                        close(client_fd);
                        free(new_client_ctx);
                    }
                }
            }
            else {
                handle_client_data(ctx);
            }
        }
    }

    free(server_ctx);
    close(server_fd);
    close(epoll_fd);
}