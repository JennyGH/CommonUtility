#include "stdafx.h"
#include "ErrorCodes.h"
#include "ErrorHandler.h"

#define errcase(code, msg)\
case code: \
{ \
	memcpy_s(buffer, sizeOfBuffer, msg, strlen(msg)); \
	break; \
}

#define defaultcase(msg)\
default: \
{\
	if(!GetWSAErrorMessage(errcode, buffer, sizeOfBuffer))\
		memcpy_s(buffer, sizeOfBuffer, msg, strlen(msg));\
	break;\
}

bool GetWSAErrorMessage(DWORD errcode, char buffer[], DWORD sizeOfBuffer)
{
    LPVOID lpMsgBuf = NULL;
    if (FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_MAX_WIDTH_MASK,
        NULL,
        errcode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&lpMsgBuf, 0,
        NULL))
    {
        sprintf_s(buffer, sizeOfBuffer, "[WSAError] %s", lpMsgBuf);
        if (NULL != lpMsgBuf)
        {
            ::LocalFree(lpMsgBuf);
        }
        return true;
    }
    if (NULL != lpMsgBuf)
    {
        ::LocalFree(lpMsgBuf);
    }
    return false;
}

void GetErrorString(UINT64 errcode, char buffer[], UINT32 sizeOfBuffer)
{
    switch (errcode)
    {
        errcase(ERROR_INVALID_CONNECTION, "Invalid connection pointer.");
        errcase(ERROR_DISCONNECTED_CONNECTION, "Disconnected connection.");
        errcase(ERROR_NO_CONNECTION_FACTORY, "No connection factory.");
        defaultcase("Unknown Error.");
    }
}

ErrorHandler::ErrorHandler(UINT64 errcode) :
    errcode(errcode)
{
    memset(errmsg, 0, sizeof(errmsg));
    GetErrorString(errcode, errmsg, sizeof(errmsg));
}
ErrorHandler::ErrorHandler(const ErrorHandler& that) :
    errcode(that.errcode)
{
    memset(errmsg, 0, sizeof(errmsg));
    memcpy_s(errmsg, sizeof(errmsg), that.errmsg, sizeof(that.errmsg));
}
ErrorHandler::~ErrorHandler() {}
