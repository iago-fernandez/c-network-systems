/**
 * @file main.c
 * @brief Application entry point with command-line argument parsing.
 */
#include "server/epoll_server.h"
#include "common/logger.h"
#include "server/signal_handler.h"

int main(int argc, char* argv[]) {
    const char* port = "8080";
    if (argc > 1) {
        port = argv[1];
    }

    logger_init(LOG_LEVEL_INFO);
    setup_signal_handlers();
    start_epoll_server(port);

    return 0;
}