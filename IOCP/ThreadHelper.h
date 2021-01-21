#pragma once
#include <cstdint>
typedef unsigned(WINAPI * thread_fn)(void*);
struct thread_t
{
    thread_fn function;
    HANDLE param;
    unsigned thread_id;
};

class ThreadHelper
{
    ThreadHelper();
public:
    ~ThreadHelper();

    static UINT32 Create(thread_fn func, UINT32 count = 1);
    static UINT32 Create(thread_fn func[], UINT32 count = 1);
    static UINT32 Create(thread_t thread, UINT32 count = 1);
    static UINT32 Create(thread_t thread[], UINT32 count = 1);
};

