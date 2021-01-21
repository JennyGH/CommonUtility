#include "stdafx.h"
#include "ThreadHelper.h"
#include <process.h>


ThreadHelper::ThreadHelper()
{
}


ThreadHelper::~ThreadHelper()
{
}

UINT32 ThreadHelper::Create(thread_fn func, UINT32 count /*= 1*/)
{
    UINT32 nCountOfSuccess = 0;

    for (UINT32 index = 0; index < count; index++)
    {
        HANDLE hHandle = (HANDLE)::_beginthreadex(NULL, 0, func, NULL, 0, NULL);
        if (hHandle != NULL)
        {
            ::CloseHandle(hHandle);
            hHandle = NULL;
            nCountOfSuccess++;
        }
    }

    return nCountOfSuccess;
}

UINT32 ThreadHelper::Create(thread_fn func[], UINT32 count /*= 1*/)
{
    UINT32 nCountOfSuccess = 0;

    for (UINT32 index = 0; index < count; index++)
    {
        HANDLE hHandle = (HANDLE)::_beginthreadex(NULL, 0, func[index], NULL, 0, NULL);
        if (hHandle != NULL)
        {
            ::CloseHandle(hHandle);
            hHandle = NULL;
            nCountOfSuccess++;
        }
    }

    return nCountOfSuccess;
}

UINT32 ThreadHelper::Create(thread_t thread, UINT32 count /*= 1*/)
{
    UINT32 nCountOfSuccess = 0;

    for (UINT32 index = 0; index < count; index++)
    {
        HANDLE hHandle = (HANDLE)::_beginthreadex(NULL, 0, thread.function, thread.param, 0, &thread.thread_id);
        if (hHandle != NULL)
        {
            ::CloseHandle(hHandle);
            hHandle = NULL;
            nCountOfSuccess++;
        }
    }

    return nCountOfSuccess;
}

UINT32 ThreadHelper::Create(thread_t thread[], UINT32 count /*= 1*/)
{
    UINT32 nCountOfSuccess = 0;

    for (UINT32 index = 0; index < count; index++)
    {
        HANDLE hHandle = (HANDLE)::_beginthreadex(NULL, 0, thread[index].function, thread[index].param, 0, &(thread[index].thread_id));
        if (hHandle != NULL)
        {
            ::CloseHandle(hHandle);
            hHandle = NULL;
            nCountOfSuccess++;
        }
    }

    return nCountOfSuccess;
}
