#include "MemoryLeakDetector.h"

MemoryLeakDetector::MemoryBlock * MemoryLeakDetector::FindBlock(void * address)
{
	MemoryLeakDetector::MemoryBlock* pBlock = m_pHead;
	while (nullptr != pBlock)
	{
		if (pBlock->address == address)
		{
			return pBlock;
		}
		pBlock = pBlock->next;
	}
	return nullptr;
}

MemoryLeakDetector::MemoryLeakDetector()
	: m_bShouldDetect(false)
	, m_nLeakMemorySize(0)
	, m_pHead(nullptr)
	, m_pTail(nullptr)
{
}

MemoryLeakDetector::~MemoryLeakDetector()
{
}

MemoryLeakDetector & MemoryLeakDetector::Get()
{
	static MemoryLeakDetector g_memoryLeakDetector;
	return g_memoryLeakDetector;
}

void MemoryLeakDetector::Start()
{
	this->m_mutex.lock();
	this->m_bShouldDetect = true;
	this->m_mutex.unlock();
}

void MemoryLeakDetector::Stop()
{
	this->m_mutex.lock();
	this->m_bShouldDetect = false;
	this->m_mutex.unlock();
}

void MemoryLeakDetector::InsertBlock(void * address, std::size_t size, const char* file, int line)
{
	this->m_mutex.lock();
	if (this->m_bShouldDetect)
	{
		MemoryLeakDetector::MemoryBlock* pBlock = (MemoryLeakDetector::MemoryBlock*)::malloc(sizeof(MemoryLeakDetector::MemoryBlock));
		memset(pBlock, 0, sizeof(MemoryLeakDetector::MemoryBlock));
		pBlock->address = address;
		pBlock->size = size;
		pBlock->line = line;
		if (nullptr != file)
		{
			int len = strlen(file);
			pBlock->filename = (char*)::malloc(sizeof(char) * (len + 1));
			memcpy(pBlock->filename, file, len);
			pBlock->filename[len] = 0;
		}
		m_nLeakMemorySize += size;
		if (nullptr == this->m_pHead || nullptr == this->m_pTail)
		{
			//首次运行
			this->m_pHead = this->m_pTail = pBlock;
		}
		else
		{
			// 找到最后一个元素
			MemoryLeakDetector::MemoryBlock* pLast = this->m_pTail;
			pBlock->prev = pLast;
			pLast->next = pBlock;
			this->m_pTail = pBlock;
		}
	}
	this->m_mutex.unlock();
}

void MemoryLeakDetector::RemoveBlock(void * address)
{
	this->m_mutex.lock();
	if (this->m_bShouldDetect)
	{
		MemoryLeakDetector::MemoryBlock* pBlock = this->FindBlock(address);
		if (nullptr != pBlock)
		{
			std::size_t size = pBlock->size;
			MemoryLeakDetector::MemoryBlock* pPrev = pBlock->prev;
			MemoryLeakDetector::MemoryBlock* pNext = pBlock->next;

			if (nullptr != pPrev)
			{
				pPrev->next = pNext;
			}
			if (nullptr != pNext)
			{
				pNext->prev = pPrev;
			}

			if (this->m_pHead == pBlock)
			{
				this->m_pHead = nullptr;
			}
			if (this->m_pTail == pBlock)
			{
				this->m_pTail = nullptr;
			}
			if (nullptr != pBlock->filename)
			{
				::free(pBlock->filename);
				pBlock->filename = nullptr;
			}
			::free(pBlock);
			m_nLeakMemorySize -= size;
		}
	}
	this->m_mutex.unlock();
}

std::size_t MemoryLeakDetector::GetLeakMemotySize() const
{
	return m_nLeakMemorySize;
}

#undef new

void* operator new(std::size_t size, void* address, const char* file, int line)
{
	void* ptr = ::malloc(size);
	if (nullptr == ptr)
	{
		printf("Out of memory\n");
		return ptr;
	}
	//printf("malloc %ld bytes, address: 0x%x\n", size, ptr);
	MemoryLeakDetector::Get().InsertBlock(ptr, size);
	return ptr;
}

void* operator new[](std::size_t size, void* ptr, const char* file, int line)
{
	return operator new(size, ptr, file, line);
}

void operator delete(void* ptr)
{
	//printf("free 0x%x\n", ptr);
	MemoryLeakDetector::Get().RemoveBlock(ptr);
	::free(ptr);
}

void operator delete[](void* ptr)
{
	operator delete(ptr);
}