#pragma once
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_ERROR 3

    void easy_logger_initialize(const char* path, int level);

    int  easy_logger_write_log(int level, const char* format, ...);

    void easy_logger_end();

#define LOG_INFO(fmt, ...)  easy_logger_write_log(LOG_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) easy_logger_write_log(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) easy_logger_write_log(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_TRACE(fmt, ...) easy_logger_write_log(LOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)
#ifdef __cplusplus
}
#endif // __cplusplus