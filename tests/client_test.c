/**
 * @file client_test.c
 * @brief Integration test client to validate server command handling and response logic.
 */
#include "common/net_utils.h"
#include "protocol/protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>

#define SERVER_PORT "8080"
#define SERVER_ADDR "127.0.0.1"
#define CMD_ECHO 0x02

int main(void) {
    printf("[TEST] Starting integration test...\n");

    // Establish connection
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1) {
        perror("[TEST] Socket creation failed");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(SERVER_PORT));

    if (inet_pton(AF_INET, SERVER_ADDR, &server_addr.sin_addr) <= 0) {
        perror("[TEST] Invalid address/ Address not supported");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[TEST] Connection failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("[TEST] Connected to server at %s:%s\n", SERVER_ADDR, SERVER_PORT);

    // Prepare Request (ECHO)
    const char* payload_data = "Integration Check: Bidirectional Flow";
    uint32_t payload_len = (uint32_t)strlen(payload_data);

    PacketHeader req_header;
    req_header.type = CMD_ECHO;
    req_header.sequence_number = 100;
    req_header.payload_length = payload_len;

    uint8_t header_buffer[sizeof(PacketHeader)];
    serialize_header(&req_header, header_buffer);

    // Send Request
    if (send(sock_fd, header_buffer, sizeof(header_buffer), 0) == -1) {
        perror("[TEST] Failed to send header");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    if (send(sock_fd, payload_data, payload_len, 0) == -1) {
        perror("[TEST] Failed to send payload");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("[TEST] Request sent (Type: %d, Len: %d). Waiting for response...\n", CMD_ECHO, payload_len);

    // Receive Response Header
    uint8_t resp_header_buf[sizeof(PacketHeader)];
    ssize_t bytes_recvd = recv(sock_fd, resp_header_buf, sizeof(resp_header_buf), 0);

    if (bytes_recvd <= 0) {
        fprintf(stderr, "[TEST] Failed to receive response header or server closed connection.\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    PacketHeader resp_header;
    deserialize_header(resp_header_buf, &resp_header);

    // Validate Header
    if (resp_header.type != CMD_ECHO) {
        fprintf(stderr, "[TEST] Error: Expected type %d, got %d\n", CMD_ECHO, resp_header.type);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    if (resp_header.payload_length != payload_len) {
        fprintf(stderr, "[TEST] Error: Expected length %d, got %d\n", payload_len, resp_header.payload_length);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // Receive Response Payload
    char* resp_payload = (char*)malloc(resp_header.payload_length + 1);
    bytes_recvd = recv(sock_fd, resp_payload, resp_header.payload_length, 0);

    if (bytes_recvd != (ssize_t)resp_header.payload_length) {
        fprintf(stderr, "[TEST] Error: Incomplete payload received.\n");
        free(resp_payload);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    resp_payload[payload_len] = '\0';

    // Validate Payload
    if (strcmp(resp_payload, payload_data) != 0) {
        fprintf(stderr, "[TEST] Error: Payload mismatch.\nSent: %s\nRecv: %s\n", payload_data, resp_payload);
        free(resp_payload);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("[TEST] Success! Server echoed: '%s'\n", resp_payload);

    free(resp_payload);
    close(sock_fd);
    return 0;
}