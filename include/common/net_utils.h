/**
 * @file net_utils.h
 * @brief Provides common network utility functions and error handling wrappers.
 */
#ifndef NET_UTILS_H
#define NET_UTILS_H

 /**
  * @brief Prints an error message and terminates the program with EXIT_FAILURE.
  *
  * @param error_message The descriptive error message to print.
  */
void die_with_error(const char* error_message);

/**
 * @brief Initializes a TCP server socket and binds it to the specified service port.
 *
 * @param service The port number or service name to bind to.
 * @return The file descriptor of the listening socket.
 */
int setup_tcp_server_socket(const char* service);

#endif