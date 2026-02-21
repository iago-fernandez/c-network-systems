/**
 * @file epoll_server.h
 * @brief Defines the asynchronous TCP server interface using epoll.
 */
#ifndef EPOLL_SERVER_H
#define EPOLL_SERVER_H

 /**
  * @brief Starts the asynchronous event loop for the server.
  *
  * @param port The port number to bind the server to.
  */
void start_epoll_server(const char* port);

#endif