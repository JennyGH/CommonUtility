#pragma once
#include <cstdint>
#include <list>
enum OverlappedOpType
{
    None = 0,   // ��״̬
    Accept,     // �����û�
    Recv,       // ��������
    Send,       // ��������
    Remove,     // �Ƴ��û�
    Disconnect, // �û��Ͽ�
};

#define MAX_BUFFER_LEN 1024
typedef struct PER_IO_CONTEXT
{
    OVERLAPPED       overlapped;               // ÿһ���ص�����������ص��ṹ(���ÿһ��Socket��ÿһ����������Ҫ��һ��)
    SOCKET           opSocket;                 // ������������ʹ�õ�Socket
    WSABUF           wsaBuf;                   // WSA���͵Ļ����������ڸ��ص�������������
    char             szBuffer[MAX_BUFFER_LEN]; // �����WSABUF�������ַ��Ļ�����
    OverlappedOpType opType;                   // ��ʶ�������������(��Ӧ�����ö��)

    PER_IO_CONTEXT()
    {
        ZeroMemory(&overlapped, sizeof(overlapped));
        ZeroMemory(szBuffer, MAX_BUFFER_LEN);
        opSocket = INVALID_SOCKET;
        wsaBuf.buf = szBuffer;
        wsaBuf.len = MAX_BUFFER_LEN;
        opType = OverlappedOpType::None;
    }

    // �ͷŵ�Socket
    ~PER_IO_CONTEXT() {}

    // ���û���������
    void ResetBuffer()
    {
        ZeroMemory(szBuffer, MAX_BUFFER_LEN);
    }

}
PER_IO_CONTEXT, *PER_IO_CONTEXT_PTR;
#undef MAX_BUFFER_LEN

typedef struct PER_SOCKET_CONTEXT
{
public:
    HANDLE                          pConnection;
    SOCKET                          socket;
private:
    CRITICAL_SECTION                criticalSection;
    std::list<PER_IO_CONTEXT_PTR>   ioContexts;

public:
    // ��ʼ��
    PER_SOCKET_CONTEXT(SOCKET socket)
    {
        ::InitializeCriticalSection(&criticalSection);
        this->socket = socket;
        this->pConnection = NULL;
    }

    // �ͷ���Դ
    ~PER_SOCKET_CONTEXT()
    {
        if (socket != INVALID_SOCKET)
        {
            ::closesocket(socket);
            socket = INVALID_SOCKET;
        }
        ::EnterCriticalSection(&criticalSection);
        std::list<PER_IO_CONTEXT_PTR>::iterator iter = ioContexts.begin();
        std::list<PER_IO_CONTEXT_PTR>::iterator end = ioContexts.end();
        for (iter; iter != end; iter++)
        {
            PER_IO_CONTEXT_PTR ptr = *iter;
            BOOL bSuccess = ::CancelIoEx((HANDLE)ptr->opSocket, &ptr->overlapped);
            if (!bSuccess)
            {
                int err = WSAGetLastError();
                if (WSA_IO_PENDING != err)
                {
                    // CancelIoEx FAILED.
                }
            }
            delete ptr;
            *iter = NULL;
        }
        ::LeaveCriticalSection(&criticalSection);
        ::DeleteCriticalSection(&criticalSection);
    }

    // ��ȡһ���µ�IoContext
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
                break;
            }
        }
        ::LeaveCriticalSection(&criticalSection);
        if (NULL != pIOContext)
        {
            delete pIOContext;
        }
    }

    std::size_t GetIOContextCount()
    {
        ::EnterCriticalSection(&criticalSection);
        std::size_t nCount = ioContexts.size();
        ::LeaveCriticalSection(&criticalSection);
        return nCount;
    }
}
PER_SOCKET_CONTEXT, *PER_SOCKET_CONTEXT_PTR;