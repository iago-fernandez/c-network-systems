/**
 * @file main.c
 * @brief Application entry point for the network server.
 */
#include "server/epoll_server.h"

int main(void) {
    start_epoll_server("8080");
    return 0;
}