#include "stdafx.h"
#include "IConnectionBase.h"


IConnectionBase::IConnectionBase() :m_pAddress(NULL)
{
}


IConnectionBase::~IConnectionBase()
{
}

void IConnectionBase::SetAddress(const void * data, UINT32 sizeOfAddress)
{
    if (data == NULL || sizeOfAddress == 0) return;
    memcpy_s(m_pAddress, GetSizeOfAddress(), data, sizeOfAddress);
}
