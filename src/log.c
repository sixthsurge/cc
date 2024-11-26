#include "log.h"
#include "common.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define COLOR_RESET   "\e[0m"
#define COLOR_BLACK   "\e[0;30m"
#define COLOR_RED     "\e[0;31m"
#define COLOR_GREEN   "\e[0;32m"
#define COLOR_YELLOW  "\e[0;33m"
#define COLOR_MAGENTA "\e[0;35m"

static struct {
    LogLevel   log_level;
    bool       write_to_stdout;
    bool       write_to_file;
    char const *log_file_path;
} log;

static char const *log_level_strings[] = {
    "",
    "trace",
    "info",
    "warning",
    "error",
};

static char const *log_level_colors[] = {
    COLOR_RESET,  // disabled
    COLOR_BLACK,  // trace
    COLOR_GREEN,  // info
    COLOR_YELLOW, // warning
    COLOR_RED,    // error
};

static bool log_initialized = false;

void log_init(
    LogLevel log_level,
    bool write_to_stdout,
    bool write_to_file,
    char const *log_file_path
) {
    log_initialized     = true;
    log.log_level       = log_level;
    log.write_to_stdout = write_to_stdout;
    log.write_to_file   = write_to_file;
    log.log_file_path   = log_file_path;
}

void vlog(
    LogLevel log_level,
    char const *format_string,
    va_list args
) {
    if (UNLIKELY(!log_initialized)) {
        fprintf(stderr, "%smust call log_init() before using logger functions%s\n", COLOR_RED, COLOR_RESET);
        exit(1);
    }

    if (log_level < log.log_level) return;

    // get time string
    char time_string[9];
    time_t now = time(NULL);
    struct tm time_info;
    localtime_r(&now, &time_info);
    strftime(time_string, sizeof time_string, "%H:%M:%S", &time_info);

    // copy args
    va_list args_copy;
    va_copy(args_copy, args);

    // write to stdout
    if (log.write_to_stdout) {
        // print heading
        printf(
            "%s%s%s [%s%s%s] ",
            COLOR_MAGENTA,
            time_string,
            COLOR_RESET,
            log_level_colors[log_level],
            log_level_strings[log_level],
            COLOR_RESET
        );
        // print message
        vprintf(format_string, args);
        // print newline
        puts("");
    }

    // write to file
    if (log.write_to_file) {
        FILE *fp = fopen(log.log_file_path, "a");

        if (fp == NULL) {
            fprintf(stderr, "%sfailed to open log file%s\n", COLOR_RED, COLOR_RESET);
            exit(1);
        }

        // print heading
        fprintf(
            fp,
            "%s [%s] ",
            time_string,
            log_level_strings[log_level]
        );
        // print message
        vfprintf(fp, format_string, args_copy);
        // print newline
        fputs("\n", fp);

        fclose(fp);
    }

    va_end(args_copy);
}

void log_trace(char const *format_string, ...) {
    va_list args;
    va_start(args, format_string);
    vlog(LOG_LEVEL_TRACE, format_string, args);
    va_end(args);
}

void log_info(char const *format_string, ...) {
    va_list args;
    va_start(args, format_string);
    vlog(LOG_LEVEL_INFO, format_string, args);
    va_end(args);
}

void log_warning(char const *format_string, ...) {
    va_list args;
    va_start(args, format_string);
    vlog(LOG_LEVEL_WARNING, format_string, args);
    va_end(args);
}

void log_error(char const *format_string, ...) {
    va_list args;
    va_start(args, format_string);
    vlog(LOG_LEVEL_ERROR, format_string, args);
    va_end(args);
}