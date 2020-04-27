#pragma once

class IOCompletionPort
{
	static unsigned WINAPI _WorkerThread(void* hHandle);
public:
	typedef void(*BeforeInitializing)(void*);
	typedef void(*AfterInitializing)(void*);
public:

	IOCompletionPort(
		void* hFileHandle,
		int nThreadCount = -1,
		int nReadBufferSize = -1,
		int nWriteBufferSize = -1
	);

	~IOCompletionPort();

	IOCompletionPort& Initialize(
		BeforeInitializing beforeInitializing = NULL,
		AfterInitializing afterInitializing = NULL
	);

	virtual void AsyncRead(void* hFileHandle) = 0;

	virtual void AsyncWrite(void* hFileHandle) = 0;

private:
	void*	m_hCompletionPort;
	void*	m_hFileHandle;
	int		m_nThreadCount;
	int		m_nReadBufferSize;
	int		m_nWriteBufferSize;
};

