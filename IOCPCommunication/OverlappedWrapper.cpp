#include "stdafx.h"
#include "OverlappedWrapper.hpp"
#define lock \
m_lock.Lock();\
common::raii::scope_member_function_noparam<CriticalSection, void> scope_lock(&m_lock, &CriticalSection::UnLock)
#define GLOBAL_LOCK \
g_lock.Lock();\
common::raii::scope_member_function_noparam<CriticalSection, void> scope_lock(&g_lock, &CriticalSection::UnLock)
//
//OverlappedWrapper::OverlappedWrapper() :
//	owner(NULL),
//	operation(AsyncOperation::None),
//	buffer(NULL)
//{
//	//memset(buffer, 0, sizeof(buffer));
//	memset(&overlapped, 0, sizeof(overlapped));
//	wsaBuffer.buf = buffer;
//	wsaBuffer.len = 0;
//}
//
//OverlappedWrapper::OverlappedWrapper(const OverlappedWrapper & that) :
//	owner(that.owner),
//	operation(that.operation),
//	buffer(NULL)
//{
//	common::raii::release_array(&buffer);
//	memcpy_s(&overlapped, sizeof(overlapped), &that.overlapped, sizeof(overlapped));
//	if (that.wsaBuffer.len <= 0) return;
//	buffer = new char[that.wsaBuffer.len]();
//	memcpy_s(buffer, that.wsaBuffer.len, that.buffer, sizeof(buffer));
//	wsaBuffer.buf = buffer;
//	wsaBuffer.len = that.wsaBuffer.len;
//}
//
//OverlappedWrapper::OverlappedWrapper(UINT32 size) :
//	owner(NULL),
//	operation(AsyncOperation::None),
//	buffer(size < 0 ? NULL : (new char[size]()))
//{
//	memset(&overlapped, 0, sizeof(overlapped));
//	wsaBuffer.buf = buffer;
//	wsaBuffer.len = (size <= 0 ? 0 : size);
//}
//
//OverlappedWrapper::~OverlappedWrapper()
//{
//	operation = AsyncOperation::None;
//	wsaBuffer.len = 0;
//	common::raii::release_array(&buffer);
//}
//
//UINT64 OverlappedWrapper::GetErrorCode() const
//{
//	DWORD dwTrans;
//	DWORD dwFlags;
//	if (WSAGetOverlappedResult(
//		owner->Socket(),
//		(WSAOVERLAPPED*)&overlapped,
//		&dwTrans,
//		FALSE,
//		&dwFlags)) return 0;
//	return WSAGetLastError();
//}
//
//OverlappedWrapper & OverlappedWrapper::NewBuffer(AsyncConnectionWrapperPtr owner, AsyncOperation op, UINT32 size)
//{
//	GLOBAL_LOCK;
//	OverlappedWrapper* buffer = new OverlappedWrapper(size);
//	buffer->owner = owner;
//	buffer->operation = op;
//	g_buffers.push_back(buffer);
//	return *g_buffers.back();
//}
//void OverlappedWrapper::RemoveBuffer(const OverlappedWrapper & buffer)
//{
//	GLOBAL_LOCK;
//	AsyncBuffers::iterator iter = g_buffers.begin();
//	AsyncBuffers::iterator end = g_buffers.end();
//	for (; iter != end; iter++)
//	{
//		if (*iter == &buffer)
//		{
//			delete *iter;
//			g_buffers.erase(iter);
//			break;
//		}
//	}
//}
