#pragma once

#include <stdarg.h>

#include "common.h"

typedef enum {
    LOG_LEVEL_DISABLED, // log disabled
    LOG_LEVEL_TRACE,    // unimportant information
    LOG_LEVEL_INFO,     // important information
    LOG_LEVEL_WARNING,  // warning
    LOG_LEVEL_ERROR,    // fatal error
} LogLevel;

/// initialize the logger
void log_init(
    LogLevel log_level,
    bool write_to_stdout,
    bool write_to_file,
    char const *log_file_path
);

/// function to write a message to the log
void vlog(
    LogLevel log_level,
    char const *format_string,
    va_list args
);

void log_trace(char const *format_string, ...) ATTRIBUTE_PRINTF_LIKE(1, 2);
void log_info(char const *format_string, ...) ATTRIBUTE_PRINTF_LIKE(1, 2);
void log_warning(char const *format_string, ...) ATTRIBUTE_PRINTF_LIKE(1, 2);
void log_error(char const *format_string, ...) ATTRIBUTE_PRINTF_LIKE(1, 2);
