#include "stdafx.h"
#include <Windows.h>
#include "ThreadHelper.h"
#include "IOCompletionPort.h"
#include "OverlappedWrapper.h"

#define DEFAULT_BUFFER_SIZE 1024

using Overlapped = OverlappedWrapper<1024>;

static UINT32								GetCountOfProcessors();

unsigned WINAPI IOCompletionPort::_WorkerThread(void * hHandle)
{
	if (NULL == hHandle)
	{
		return ERROR_BAD_ARGUMENTS;
	}

	IOCompletionPort* pThis = (IOCompletionPort*)hHandle;

	while (true)
	{
		DWORD nBytesOfTransferred = 0;
		Overlapped* lpOverlapped = NULL;
		BOOL bSuccess = ::GetQueuedCompletionStatus(
			pThis->m_hCompletionPort,
			&nBytesOfTransferred,
			NULL,
			//(PULONG_PTR)&lpCompletionKey,
			(LPOVERLAPPED*)&lpOverlapped,
			WSA_INFINITE);

		if (-1 == nBytesOfTransferred)
		{
			//Exit
			break;
		}

		if (NULL == lpOverlapped)
		{
			continue;
		}

		//Free OverlappedWrapper pointer while going out of this scope.
		common::raii::scope_function<void, Overlapped**>
			scope_release_overlapped(
				common::raii::release_object,
				&lpOverlapped);

		Overlapped::OverlappedOperation operation = lpOverlapped->GetOperation();
		switch (operation)
		{
		case Overlapped::None:
			break;
		case Overlapped::Read:
			break;
		case Overlapped::Write:
			break;
		default:
			break;
		}

	}

	return 0;
}

IOCompletionPort::IOCompletionPort(void* hFileHandle, int nThreadCount, int nReadBufferSize, int nWriteBufferSize)
	: m_hCompletionPort(::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0))
	, m_hFileHandle(hFileHandle)
	, m_nThreadCount(nThreadCount > 0 ? nThreadCount : GetCountOfProcessors())
	, m_nReadBufferSize(nReadBufferSize > 0 ? nReadBufferSize : DEFAULT_BUFFER_SIZE)
	, m_nWriteBufferSize(nWriteBufferSize > 0 ? nWriteBufferSize : DEFAULT_BUFFER_SIZE)
{
}


IOCompletionPort::~IOCompletionPort()
{
	if (NULL != m_hCompletionPort)
	{
		::CloseHandle(m_hCompletionPort);
		m_hCompletionPort = NULL;
	}
}

IOCompletionPort & IOCompletionPort::Initialize(BeforeInitializing fnBeforeInitializing, AfterInitializing fnAfterInitializing)
{
	if (NULL != fnBeforeInitializing)
	{
		fnBeforeInitializing(m_hFileHandle);
	}

	//创建完成端口
	::CreateIoCompletionPort((HANDLE)m_hFileHandle, m_hCompletionPort, 0, 0);

	if (NULL != fnAfterInitializing)
	{
		fnAfterInitializing(m_hFileHandle);
	}

	//创建若干个工作线程
	thread_t thread = { _WorkerThread, this };
	ThreadHelper::Create(thread, m_nThreadCount);

	return *this;
}

static UINT32 GetCountOfProcessors()
{
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	return si.dwNumberOfProcessors;
}