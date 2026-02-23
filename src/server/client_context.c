/**
 * @file client_context.c
 * @brief Implementation of client context lifecycle management.
 */
#include "server/client_context.h"
#include <stdlib.h>
#include <string.h>

void init_client_context(ClientContext* ctx, int fd) {
    memset(ctx, 0, sizeof(ClientContext));
    ctx->fd = fd;
    ctx->state = STATE_READING_HEADER;
}

void reset_client_context(ClientContext* ctx) {
    if (ctx->payload_buffer != NULL) {
        free(ctx->payload_buffer);
        ctx->payload_buffer = NULL;
    }

    ctx->state = STATE_READING_HEADER;
    ctx->header_bytes_read = 0;
    ctx->payload_bytes_read = 0;
    ctx->expected_payload_length = 0;
    // We do not reset fd, as the connection is still active
    memset(ctx->header_buffer, 0, sizeof(ctx->header_buffer));
}

void free_client_context(ClientContext* ctx) {
    reset_client_context(ctx);
}