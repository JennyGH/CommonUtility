#pragma once
#include <Windows.h>
class CriticalSection
{
public:
	CriticalSection();
	~CriticalSection();

	void Lock();
	void UnLock();

private:
	CRITICAL_SECTION m_lock;
};

