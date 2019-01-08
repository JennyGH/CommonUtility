#pragma once
#include "Globals.h"
#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#endif // 

namespace common
{
	namespace async
	{
		template<typename _owner_type, typename _op_type>
		class AsyncBuffer : OVERLAPPED
		{
		public:
			AsyncBuffer() {}
			AsyncBuffer(_op_type op) :m_op(op) {}
			AsyncBuffer(_owner_type owner, _op_type op) : m_owner(owner), m_op(op) {}
			~AsyncBuffer() {}
		private:
			_owner_type m_owner;
			_op_type m_op;
		};
	}
}


namespace common
{
	namespace async
	{
		template<typename _Buffer, unsigned long _timeout = INFINITE, int _needThreads = 4>
		class CompletionPort
		{
		public:
			typedef void(WINAPI * complete_callback_t)(DWORD, HANDLE, _Buffer*);
			typedef void(WINAPI * exception_callback_t)(DWORD);
		private:
			static DWORD WINAPI _worker_thread(HANDLE hHandle)
			{
				if (null == hHandle)
				{
					return -1UL;
				}
				CompletionPort& self = *((CompletionPort*)hHandle);
				while (true)
				{
					HANDLE pCompeltionKey = null;
					DWORD nBytesOfTransferred = 0; // Bytes of transfered in completion port.
					_Buffer* pOverlapped = null;
					BOOL bSuccess = ::GetQueuedCompletionStatus(self.m_hCompletionPort, &nBytesOfTransferred, (PULONG_PTR)&pCompeltionKey, (LPOVERLAPPED*)&pOverlapped, _timeout);
					if (bSuccess)
					{
						self.onComplete(nBytesOfTransferred, pCompeltionKey, pOverlapped);
					}
					else
					{
						// Exception occurred.
						self.onException(GetLastError(), pCompeltionKey, pOverlapped);
					}
				}

				return 0UL;
			}
		public:
			CompletionPort() :
				m_hCompletionPort(null),
				m_nCountOfThread(0)
			{
				MY_ASSERT(_needThreads >= 1);
			}
			virtual ~CompletionPort()
			{
				close();
			}

			CompletionPort& initialize()
			{
				// Create completion port.
				::CreateIoCompletionPort(m_hFileHandle, m_hCompletionPort, null, 0);

				// Create worker threads.
				for (int index = 0; index < _needThreads; index++)
				{
					HANDLE hThread = ::CreateThread(null, 0, _worker_thread, this, 0, null);
					if (null != hThread)
					{
						::CloseHandle(hThread);
						hThread = null;
						m_nCountOfThread++;
					}
				}
				return *this;
			}

			CompletionPort& bind(HANDLE hFileHandle)
			{
				if (hFileHandle != null)
				{
					::CreateIoCompletionPort(hFileHandle, m_hCompletionPort, null, 0);
				}
				return *this;
			}

			CompletionPort& close()
			{
				::CloseHandle(m_hCompletionPort);
				m_hCompletionPort = null;
			}

		protected:
			virtual void onComplete(DWORD nBytesOfTransfered, HANDLE hCompletionKey, _Buffer* pBuffer) = 0;
			virtual void onException(DWORD nErrorCode, HANDLE hCompletionKey, _Buffer* pBuffer) = 0;
		protected:
			HANDLE m_hCompletionPort;
			HANDLE m_hFileHandle;
			unsigned int m_nCountOfThread;
		};
	}
}