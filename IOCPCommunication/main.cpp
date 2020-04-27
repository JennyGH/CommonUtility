// IOCPCommunication.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "IOCPCommunication.h"

#define CONSOLE(fmt, ...) printf(fmt "\r\n", ## __VA_ARGS__)

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
	public IOCPCommunication
{
public:
	AsyncCommunication(const IOCPSettings& settings) : IOCPCommunication(settings, new NetworkConnectionFactory()) {}
	~AsyncCommunication() {}

private:
	virtual void OnAccepted(const IConnectionBase*, const BYTE data[], UINT64 len)
	{
		CONSOLE("=============== OnAccepted ===============");
		CONSOLE("%s", data);
	};
	virtual void OnRecved(const IConnectionBase*, const BYTE data[], UINT64 len)
	{
		CONSOLE("=============== OnRecved ===============");
		CONSOLE("%s", data);
	};
	virtual void OnSent(const IConnectionBase*, const BYTE data[], UINT64 len)
	{
		CONSOLE("=============== OnSent ===============");
		CONSOLE("%s", data);
	};
	virtual void OnError(const IConnectionBase*, UINT64 errcode)
	{
		CONSOLE("=============== OnError ===============");
		CONSOLE("Error code: %d", errcode);
	};
	virtual void OnRemoved(const IConnectionBase*)
	{
		CONSOLE("=============== OnRemoved ===============");
	};
	virtual void OnClientLost(const IConnectionBase*)
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
	settings.isAcceptWithData = true;

	AsyncCommunication async_server(settings);

	async_server.Startup(&address, sizeof(sockaddr_in));

	_CrtDumpMemoryLeaks();

	while (true)
	{
		::Sleep(1000);
	}

	return 0;
}
