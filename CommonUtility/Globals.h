#pragma once
#ifndef null
#if __cplusplus <= 199711L
#define null NULL
#else
#define null nullptr
#endif
#endif // !null

#ifndef WINAPI
#if !defined(WIN32) || !defined(_WIN32)
#define WINAPI __stdcall
#else
#define WINAPI 
#endif
#endif // !WINAPI
#include <assert.h>
#define MY_ASSERT assert