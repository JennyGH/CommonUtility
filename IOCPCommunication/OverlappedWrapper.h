#pragma once
/*
 * 对重叠IO结构的包装
 */
template<int _BufferSize>
class OverlappedWrapper
{
	OverlappedWrapper(const OverlappedWrapper&);
	OverlappedWrapper& operator= (const OverlappedWrapper);

public:
	enum OverlappedOperation
	{
		None = 0,
		Read,
		Write
	};

public:
	OverlappedWrapper(OverlappedOperation operation);
	~OverlappedWrapper();

	OverlappedOperation GetOperation() const;

public:
	OVERLAPPED overlapped;
	WSABUF wsaBuffer;

private:
	OverlappedOperation m_operation;
	char m_buffer[_BufferSize];
};

template<int _BufferSize>
inline OverlappedWrapper<_BufferSize>::OverlappedWrapper(OverlappedOperation operation)
	:operation(OverlappedOperation::None)
{
	memset(&overlapped, 0, sizeof(overlapped));
	wsaBuffer.buf = m_buffer;
	wsaBuffer.len = (_BufferSize <= 0 ? 0 : _BufferSize);
}

template<int _BufferSize>
inline OverlappedWrapper<_BufferSize>::~OverlappedWrapper()
{
}

template<int _BufferSize>
inline OverlappedWrapper<_BufferSize>::OverlappedOperation OverlappedWrapper<_BufferSize>::GetOperation() const
{
	return this->m_operation;
}
