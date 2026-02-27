/**
 * @file signal_handler.c
 * @brief Implementation of POSIX signal handlers.
 */
#include "server/signal_handler.h"
#include "common/logger.h"
#include <stddef.h>

volatile sig_atomic_t server_running = 1;

// Handles the incoming POSIX signals.
static void handle_signal(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        server_running = 0;
    }
}

void setup_signal_handlers(void) {
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        LOG_ERROR("Failed to register SIGINT handler.");
    }

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        LOG_ERROR("Failed to register SIGTERM handler.");
    }
}