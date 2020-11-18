#pragma once
#include <cstdint>
#include <list>
enum OverlappedOpType
{
    None = 0,
    Accept,
    Recv,
    Send,
};

#define MAX_BUFFER_LEN 1024
typedef struct PER_IO_CONTEXT
{
    OVERLAPPED       overlapped;               // 每一个重叠网络操作的重叠结构(针对每一个Socket的每一个操作，都要有一个)
    SOCKET           opSocket;                 // 这个网络操作所使用的Socket
    WSABUF           wsaBuf;                   // WSA类型的缓冲区，用于给重叠操作传参数的
    char             szBuffer[MAX_BUFFER_LEN]; // 这个是WSABUF里具体存字符的缓冲区
    OverlappedOpType opType;                   // 标识网络操作的类型(对应上面的枚举)

    PER_IO_CONTEXT()
    {
        ZeroMemory(&overlapped, sizeof(overlapped));
        ZeroMemory(szBuffer, MAX_BUFFER_LEN);
        opSocket = INVALID_SOCKET;
        wsaBuf.buf = szBuffer;
        wsaBuf.len = MAX_BUFFER_LEN;
        opType = OverlappedOpType::None;
    }

    // 释放掉Socket
    ~PER_IO_CONTEXT() {}

    // 重置缓冲区内容
    void ResetBuffer()
    {
        ZeroMemory(szBuffer, MAX_BUFFER_LEN);
    }

}
PER_IO_CONTEXT, *PER_IO_CONTEXT_PTR;
#undef MAX_BUFFER_LEN

typedef struct PER_SOCKET_CONTEXT
{
    HANDLE                          pConnection;
    SOCKET                          socket;
    std::list<PER_IO_CONTEXT_PTR>   ioContexts;
    CRITICAL_SECTION                criticalSection;

    // 初始化
    PER_SOCKET_CONTEXT(SOCKET socket, void* pConnection)
    {
        ::InitializeCriticalSection(&criticalSection);
        this->socket = socket;
        this->pConnection = pConnection;
    }

    // 释放资源
    ~PER_SOCKET_CONTEXT()
    {
        if (socket != INVALID_SOCKET)
        {
            ::closesocket(socket);
            socket = INVALID_SOCKET;
        }
        ::DeleteCriticalSection(&criticalSection);
    }

    // 获取一个新的IoContext
    PER_IO_CONTEXT_PTR NewIOContext(OverlappedOpType op)
    {
        PER_IO_CONTEXT_PTR pIOContext = new PER_IO_CONTEXT();
        pIOContext->opType = op;
        pIOContext->opSocket = socket;

        ::EnterCriticalSection(&criticalSection);
        ioContexts.push_back(pIOContext);
        ::LeaveCriticalSection(&criticalSection);

        return pIOContext;
    }

    void RemoveContext(PER_IO_CONTEXT_PTR pIOContext)
    {
        ::EnterCriticalSection(&criticalSection);
        std::list<PER_IO_CONTEXT_PTR>::iterator iter = ioContexts.begin();
        std::list<PER_IO_CONTEXT_PTR>::iterator end = ioContexts.end();
        for (iter; iter != end; iter++)
        {
            if (*iter == pIOContext)
            {
                ioContexts.erase(iter);
                delete pIOContext;
                break;
            }
        }
        ::LeaveCriticalSection(&criticalSection);
    }
}
PER_SOCKET_CONTEXT, *PER_SOCKET_CONTEXT_PTR;
