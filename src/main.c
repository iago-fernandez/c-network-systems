/**
 * @file main.c
 * @brief Application entry point.
 */
#include "server/epoll_server.h"
#include "common/logger.h"
#include "server/signal_handler.h"

int main(void) {
    logger_init(LOG_LEVEL_INFO);
    setup_signal_handlers();
    start_epoll_server("8080");
    return 0;
}