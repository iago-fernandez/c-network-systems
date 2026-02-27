/**
 * @file signal_handler.c
 * @brief Implementation of POSIX signal handlers.
 */
#include "server/signal_handler.h"
#include "common/logger.h"
#include <stddef.h>
#include <unistd.h>

volatile sig_atomic_t server_running = 1;

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

    signal(SIGPIPE, SIG_IGN);
}