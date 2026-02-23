/**
 * @file client_context.h
 * @brief Defines the client context structure and state machine for non-blocking I/O.
 */
#ifndef CLIENT_CONTEXT_H
#define CLIENT_CONTEXT_H

#include <stdint.h>
#include <stddef.h>
#include "protocol/protocol.h"

 /**
  * @brief Represents the current state of the client connection.
  */
typedef enum {
    STATE_READING_HEADER,
    STATE_READING_PAYLOAD,
    STATE_PROCESSING
} ClientState;

/**
 * @brief Holds the state and buffers for a specific client connection.
 */
typedef struct {
    int fd;
    ClientState state;

    uint8_t header_buffer[8];
    size_t header_bytes_read;

    uint8_t* payload_buffer;
    uint32_t expected_payload_length;
    size_t payload_bytes_read;

    uint16_t message_type;
} ClientContext;

/**
 * @brief Initializes a new client context.
 *
 * @param ctx Pointer to the context to initialize.
 * @param fd The file descriptor associated with the client.
 */
void init_client_context(ClientContext* ctx, int fd);

/**
 * @brief Resets the client context for the next message, freeing payload memory.
 *
 * @param ctx Pointer to the context to reset.
 */
void reset_client_context(ClientContext* ctx);

/**
 * @brief Frees all resources associated with the client context.
 *
 * @param ctx Pointer to the context to free.
 */
void free_client_context(ClientContext* ctx);

#endif