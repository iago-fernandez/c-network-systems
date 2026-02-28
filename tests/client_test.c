/**
 * @file client_test.c
 * @brief Integration test client to validate bidirectional binary protocol communication.
 */
#include "protocol/protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define CMD_ECHO 0x02

int main(int argc, char* argv[]) {
    int sock_fd;
    struct sockaddr_in server_addr;
    const char* port_str = "8080";

    if (argc > 1) {
        port_str = argv[1];
    }
    int server_port = atoi(port_str);

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("[TEST] Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("[TEST] Invalid address/ Address not supported");
        return 1;
    }

    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("[TEST] Connection failed");
        return 1;
    }
    printf("[TEST] Connected to server on port %d.\n", server_port);

    const char* message_data = "Integration Check: Bidirectional Flow";
    uint32_t data_len = (uint32_t)strlen(message_data);

    PacketHeader req_header;
    memset(&req_header, 0, sizeof(PacketHeader));
    req_header.type = CMD_ECHO;
    req_header.sequence_number = 1;
    req_header.payload_length = data_len;

    uint8_t buffer[sizeof(PacketHeader)];
    serialize_header(&req_header, buffer);

    if (send(sock_fd, buffer, sizeof(buffer), 0) != sizeof(buffer)) {
        perror("[TEST] Failed to send header");
        close(sock_fd);
        return 1;
    }

    if (send(sock_fd, message_data, data_len, 0) != (ssize_t)data_len) {
        perror("[TEST] Failed to send payload");
        close(sock_fd);
        return 1;
    }
    printf("[TEST] Request sent (Type: %d, Len: %d). Waiting for response...\n", CMD_ECHO, data_len);

    uint8_t resp_header_buf[sizeof(PacketHeader)];
    if (recv(sock_fd, resp_header_buf, sizeof(resp_header_buf), 0) <= 0) {
        fprintf(stderr, "[TEST] Failed to receive response header.\n");
        close(sock_fd);
        return 1;
    }

    PacketHeader resp_header;
    deserialize_header(resp_header_buf, &resp_header);

    char* resp_payload = (char*)malloc(resp_header.payload_length + 1);
    if (recv(sock_fd, resp_payload, resp_header.payload_length, 0) != (ssize_t)resp_header.payload_length) {
        fprintf(stderr, "[TEST] Incomplete payload received.\n");
        free(resp_payload);
        close(sock_fd);
        return 1;
    }
    resp_payload[resp_header.payload_length] = '\0';

    if (strcmp(resp_payload, message_data) == 0) {
        printf("[TEST] Success! Server echoed: '%s'\n", resp_payload);
    }
    else {
        printf("[TEST] Failure! Payload mismatch.\n");
    }

    free(resp_payload);
    close(sock_fd);
    return 0;
}