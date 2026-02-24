/**
 * @file logger.h
 * @brief Defines the logging interface for the application.
 */
#ifndef LOGGER_H
#define LOGGER_H

 /**
  * @enum LogLevel
  * @brief Defines the severity levels for log messages.
  */
typedef enum {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL
} LogLevel;

/**
 * @brief Initializes the logging subsystem.
 * @param level The minimum log level to display.
 */
void logger_init(LogLevel level);

/**
 * @brief Logs a formatted message with a timestamp and severity level.
 * @param level The severity level of the message.
 * @param file The source file name generating the log.
 * @param line The line number in the source file.
 * @param fmt The format string (printf-style).
 */
void logger_log(LogLevel level, const char* file, int line, const char* fmt, ...);

// Convenience macros to automatically capture file and line number
#define LOG_DEBUG(...) logger_log(LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...)  logger_log(LOG_LEVEL_INFO,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  logger_log(LOG_LEVEL_WARN,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) logger_log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) logger_log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#endif