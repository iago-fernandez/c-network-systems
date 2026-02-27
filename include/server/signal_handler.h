/**
 * @file signal_handler.h
 * @brief Defines the POSIX signal handling interface for the server.
 */
#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>

extern volatile sig_atomic_t server_running;

/**
 * @brief Sets up handlers for SIGINT and SIGTERM to allow graceful shutdown.
 */
void setup_signal_handlers(void);

#endif