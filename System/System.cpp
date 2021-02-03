#include "stdafx.h"
#include "System.h"

#if WIN32
#include <Windows.h>
#else
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#endif // WIN32

int System::GetLastError()
{
#if WIN32
    return ::GetLastError();
#else
    return errno;
#endif // WIN32
}

std::string System::GetErrorMessage(int errcode)
{
    std::string res;
#if WIN32
    LPVOID lpMsgBuf = NULL;
    DWORD  dwLength = ::FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errcode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&lpMsgBuf,
        0,
        NULL
    );
    if (NULL != lpMsgBuf)
    {
        res.assign((LPCSTR)lpMsgBuf, dwLength);
        ::LocalFree(lpMsgBuf);
        lpMsgBuf = NULL;
        std::size_t length = res.size();
        while (length && (res[length - 1] == '\n' || res[length - 1] == '\r'))
        {
            res.erase(res.size() - 1);
            length--;
        }
    }
#else
    char buffer[1024] = { 0 };
    ::strerror_r(errcode, buffer, sizeof(buffer));
    res.assign(buffer);
#endif // WIN32
    return res;
}

std::string System::GetLastErrorMessage()
{
    return System::GetErrorMessage(System::GetLastError());
}
