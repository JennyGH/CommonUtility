#pragma once
#include <cstdint>
#include <string>
class IConnectionBase
{
public:
    IConnectionBase();
    virtual ~IConnectionBase();
    virtual std::string GetAddress() const = 0;
    virtual UINT32 GetAddressFamily() const = 0;
    virtual UINT32 GetSizeOfAddress() const = 0;
    void SetAddress(const void* data, UINT32 sizeOfAddress);

protected:
    void * m_pAddress;
};

class IConnectionFactory
{
public:
    virtual IConnectionBase* NewConnection() = 0;
};


