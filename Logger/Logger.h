#pragma once
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#ifndef STDCALL
#if defined(WIN32) || defined(_WIN32)
#define STDCALL __stdcall
#else
#define STDCALL
#endif // defined(WIN32) || defined(_WIN32)
#endif // !STDCALL

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO  2
#define LOG_LEVEL_ERROR 3

#define EASY_LOGGER_SUCCESS                      1
#define EASY_LOGGER_ERROR_FAIL                   0
#define EASY_LOGGER_ERROR_INVALID_ARGUMENT      -1
#define EASY_LOGGER_ERROR_NO_ACCESS_RIGHT       -2

    int easy_logger_initialize(int level, FILE* pFile);

    int easy_logger_set_indent_size(int size);

    int easy_logger_set_indent_content(const char* content);

    int easy_logger_enter_group(const char* group);

    int easy_logger_leave_group();

    int easy_logger_write_log(int level, const char* format, ...);

    int easy_logger_close();

#define LOG_INFO(fmt, ...)  easy_logger_write_log(LOG_LEVEL_INFO,  fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) easy_logger_write_log(LOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) easy_logger_write_log(LOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#ifdef __cplusplus
}
#endif // __cplusplus