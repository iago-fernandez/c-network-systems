/**
 * @file client_test.c
 * @brief Simple TCP client to validate binary protocol communication.
 */
#include "protocol/protocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080

int main(void) {
    int sock_fd;
    struct sockaddr_in server_addr;

    // Create TCP socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return 1;
    }

    // Establish connection to the server
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return 1;
    }
    printf("Connected to server.\n");

    // Prepare the payload data
    const char* message_data = "Hello, High Performance Server!";
    uint32_t data_len = (uint32_t)strlen(message_data);

    ProtocolHeader header;
    header.payload_length = data_len;
    header.message_type = MSG_TYPE_DATA;
    header.version = PROTOCOL_VERSION;

    // Serialize header to network byte order
    uint8_t buffer[8];
    serialize_header(&header, buffer);

    // Send the fixed-size header
    if (send(sock_fd, buffer, sizeof(buffer), 0) != sizeof(buffer)) {
        perror("Failed to send header");
        close(sock_fd);
        return 1;
    }
    printf("Header sent (Len: %d, Type: %d).\n", data_len, header.message_type);

    // Send the payload body
    if (send(sock_fd, message_data, data_len, 0) != (ssize_t)data_len) {
        perror("Failed to send payload");
        close(sock_fd);
        return 1;
    }
    printf("Payload sent: %s\n", message_data);

    // Cleanup resources
    close(sock_fd);
    return 0;
}