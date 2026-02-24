/**
 * @file logger.c
 * @brief Implementation of the application logging system.
 */
#include "common/logger.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static LogLevel current_level = LOG_LEVEL_INFO;

static const char* level_strings[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char* level_colors[] = {
    "\x1b[36m", // DEBUG - Cyan
    "\x1b[32m", // INFO  - Green
    "\x1b[33m", // WARN  - Yellow
    "\x1b[31m", // ERROR - Red
    "\x1b[35m"  // FATAL - Magenta
};

void logger_init(LogLevel level) {
    current_level = level;
}

void logger_log(LogLevel level, const char* file, int line, const char* fmt, ...) {
    if (level < current_level) return;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    char time_buf[20];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", t);

    // Header with color
    fprintf(stderr, "%s[%s] [%-5s] %s:%d: \x1b[0m",
        level_colors[level], time_buf, level_strings[level], file, line);

    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}