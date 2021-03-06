// IOCP.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "IOCP.h"

#define CONSOLE(fmt, ...) printf(fmt "\r\n", ## __VA_ARGS__)

static std::string GenerateHttpResponse()
{
    return
        "HTTP/1.1 200 OK\r\n"
        "Content-Type：text/html\r\n"
        "\r\n"
        "<body>\r\n"
        "   <h1>Hello World!</h1>\r\n"
        "</body>"
        ;
}

class NetworkConnection :
    public IConnectionBase
{
public:
    NetworkConnection()
    {
        m_pAddress = &addr;
        memset(m_pAddress, 0, sizeof(addr));
    }
    ~NetworkConnection() {}
    virtual std::string GetAddress() const override
    {
        if (m_pAddress == NULL) return "";
        return inet_ntoa(addr.sin_addr);
    }
    virtual UINT32 GetAddressFamily() const override
    {
        if (m_pAddress == NULL) return -1;
        return addr.sin_family;
    }
    virtual UINT32 GetSizeOfAddress() const override
    {
        return sizeof(addr);
    }
private:
    SOCKADDR_IN addr;
};
class NetworkConnectionFactory :
    public IConnectionFactory
{
public:
    virtual IConnectionBase* NewConnection() override { return new NetworkConnection(); }

};


class AsyncCommunication :
    public IOCP
{
public:
    AsyncCommunication(const IOCPSettings& settings) : IOCP(new NetworkConnectionFactory(), settings) {}
    ~AsyncCommunication() {}

private:
    virtual void OnAccepted(IConnectionBase* pConnection, const BYTE data[], UINT64 len)
    {
        CONSOLE("=============== OnAccepted ===============");
        CONSOLE("%s", data);
        if (len > 0)
        {
            std::string response = GenerateHttpResponse();
            this->Send(pConnection, (const unsigned char*)response.c_str(), response.length());
        }
    };
    virtual void OnRecved(IConnectionBase* pConnection, const BYTE data[], UINT64 len)
    {
        CONSOLE("=============== OnRecved ===============");
        CONSOLE("%s", data);
        std::string response = GenerateHttpResponse();
        this->Send(pConnection, (const unsigned char*)response.c_str(), response.length());
    };
    virtual void OnSent(IConnectionBase*, const BYTE data[], UINT64 len)
    {
        CONSOLE("=============== OnSent ===============");
        CONSOLE("%s", data);
    };
    virtual void OnError(IConnectionBase*, INT32 errcode)
    {
        CONSOLE("=============== OnError ===============");
        CONSOLE("Error code: %d", errcode);
    };
    virtual void OnRemoved(IConnectionBase*)
    {
        CONSOLE("=============== OnRemoved ===============");
    };
    virtual void OnClientLost(IConnectionBase*)
    {
        CONSOLE("=============== OnClientLost ===============");
    };

};

int main()
{
    sockaddr_in address;
    address.sin_port = htons(9999);
    address.sin_addr.s_addr = inet_addr("0.0.0.0");

    IOCPSettings settings = DEFAULT_IOCP_SETTINGS;
    //settings.nCountOfThreads = 1;
    settings.isAcceptWithData = false;

    AsyncCommunication async_server(settings);

    async_server.Start(&address, sizeof(sockaddr_in));

    _CrtDumpMemoryLeaks();

    while (true)
    {
        ::Sleep(1000);
    }

    return 0;
}
