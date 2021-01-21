#include "StdAfx.h"
#include "CriticalSection.h"


CriticalSection::CriticalSection()
{
    InitializeCriticalSection(&m_lock);
}


CriticalSection::~CriticalSection()
{
    ::DeleteCriticalSection(&m_lock);
}

void CriticalSection::Lock()
{
    EnterCriticalSection(&m_lock);
}

void CriticalSection::UnLock()
{
    LeaveCriticalSection(&m_lock);
}
