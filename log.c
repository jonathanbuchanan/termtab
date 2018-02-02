#include "log.h"

#include <stdio.h>
#include <stdarg.h>

char *log_filename;

void open_log_file(char *filename) {
    log_filename = filename;
    FILE *log_file = fopen(log_filename, "w");
    fclose(log_file);
}

void _log(char *filename, int line, char *format, ...) {
    char buffer[1024];

    va_list args;
    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);

    FILE *log_file = fopen(log_filename, "a");
    fprintf(log_file, "[%s:%d] %s\n", filename, line, buffer);
    fclose(log_file);
}
