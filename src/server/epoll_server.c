/**
 * @file epoll_server.c
 * @brief Implementation of the asynchronous epoll event loop with context management.
 */
#include "server/epoll_server.h"
#include "server/client_context.h"
#include "common/net_utils.h"
#include "protocol/protocol.h"
#include "common/logger.h"
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
#define CMD_ECHO 0x02

 // Configures a file descriptor to operate in non-blocking mode.
static void set_non_blocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        die_with_error("fcntl F_GETFL failed");
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        die_with_error("fcntl F_SETFL failed");
    }
}

// Helper function to serialize and send a response packet.
static void send_response(int fd, uint8_t type, const uint8_t* payload, uint32_t payload_len) {
    PacketHeader header;
    header.type = type;
    header.sequence_number = 0;
    header.payload_length = payload_len;

    ssize_t sent = send(fd, &header, sizeof(header), 0);
    if (sent == -1) {
        LOG_ERROR("Failed to send response header: %s", strerror(errno));
        return;
    }

    if (payload_len > 0 && payload != NULL) {
        sent = send(fd, payload, payload_len, 0);
        if (sent == -1) {
            LOG_ERROR("Failed to send response payload: %s", strerror(errno));
        }
    }
}

// Handles data reading for a connected client using a state machine.
static void handle_client_data(ClientContext* ctx) {
    ssize_t bytes_read;

    while (1) {
        if (ctx->state == STATE_READING_HEADER) {
            size_t remaining = sizeof(ctx->header_buffer) - ctx->header_bytes_read;
            bytes_read = recv(ctx->fd, ctx->header_buffer + ctx->header_bytes_read, remaining, 0);

            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                LOG_ERROR("recv header failed: %s", strerror(errno));
                close(ctx->fd);
                free_client_context(ctx);
                free(ctx);
                return;
            }
            else if (bytes_read == 0) {
                LOG_INFO("Client fd %d disconnected during header read.", ctx->fd);
                close(ctx->fd);
                free_client_context(ctx);
                free(ctx);
                return;
            }

            ctx->header_bytes_read += bytes_read;

            if (ctx->header_bytes_read == sizeof(ctx->header_buffer)) {
                PacketHeader header;
                deserialize_header(ctx->header_buffer, &header);

                ctx->expected_payload_length = header.payload_length;
                ctx->message_type = header.type;

                if (ctx->expected_payload_length > 0) {
                    if (ctx->expected_payload_length > MAX_PAYLOAD_SIZE) {
                        LOG_WARN("Payload too large: %d", ctx->expected_payload_length);
                        close(ctx->fd);
                        free_client_context(ctx);
                        free(ctx);
                        return;
                    }

                    ctx->payload_buffer = (uint8_t*)malloc(ctx->expected_payload_length);
                    if (!ctx->payload_buffer) {
                        LOG_ERROR("malloc payload failed: %s", strerror(errno));
                        close(ctx->fd);
                        free(ctx);
                        return;
                    }
                    ctx->state = STATE_READING_PAYLOAD;
                }
                else {
                    LOG_DEBUG("Received header-only message. Type: %d", header.type);
                    reset_client_context(ctx);
                }
            }
        }
        else if (ctx->state == STATE_READING_PAYLOAD) {
            size_t remaining = ctx->expected_payload_length - ctx->payload_bytes_read;
            bytes_read = recv(ctx->fd, ctx->payload_buffer + ctx->payload_bytes_read, remaining, 0);

            if (bytes_read == -1) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                LOG_ERROR("recv payload failed: %s", strerror(errno));
                close(ctx->fd);
                free_client_context(ctx);
                free(ctx);
                return;
            }
            else if (bytes_read == 0) {
                LOG_INFO("Client fd %d disconnected during payload read.", ctx->fd);
                close(ctx->fd);
                free_client_context(ctx);
                free(ctx);
                return;
            }

            ctx->payload_bytes_read += bytes_read;

            if (ctx->payload_bytes_read == ctx->expected_payload_length) {
                LOG_INFO("Processing command type: %d, Length: %d", ctx->message_type, ctx->expected_payload_length);

                switch (ctx->message_type) {
                case CMD_ECHO:
                    LOG_INFO("Executing ECHO command.");
                    send_response(ctx->fd, CMD_ECHO, ctx->payload_buffer, ctx->expected_payload_length);
                    break;
                default:
                    LOG_WARN("Unknown command type: %d", ctx->message_type);
                    break;
                }

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

    LOG_INFO("Server listening on port %s (Binary Protocol V1)...", port);

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
                            LOG_ERROR("accept failed: %s", strerror(errno));
                            break;
                        }
                    }

                    char client_ip[INET_ADDRSTRLEN];
                    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                    LOG_INFO("Accepted connection from %s:%d (fd: %d).", client_ip, ntohs(client_addr.sin_port), client_fd);

                    set_non_blocking(client_fd);

                    ClientContext* new_client_ctx = (ClientContext*)malloc(sizeof(ClientContext));
                    init_client_context(new_client_ctx, client_fd);

                    event.data.ptr = new_client_ctx;
                    event.events = EPOLLIN | EPOLLET;

                    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                        LOG_ERROR("epoll_ctl EPOLL_CTL_ADD client failed: %s", strerror(errno));
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