#ifndef LOG_H
#define LOG_H

void open_log_file(char *filename);

void _log(char *filename, int line, char *format, ...);
#define LOG(...) _log(__FILE__, __LINE__, __VA_ARGS__)

#endif
