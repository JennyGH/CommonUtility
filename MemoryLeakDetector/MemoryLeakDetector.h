#pragma once
#include <mutex>
#include <atomic>
#include <string>

class MemoryLeakDetector
{
	struct MemoryBlock
	{
		void* address;
		std::size_t size;
		char* filename;
		int line;
		MemoryBlock* prev;
		MemoryBlock* next;
	};

	MemoryBlock* FindBlock(void* address);

	MemoryLeakDetector();
public:
	~MemoryLeakDetector();

	static MemoryLeakDetector& Get();

	void InsertBlock(void* address, std::size_t size, const char* file = nullptr, int line = 0);

	void RemoveBlock(void* address);

	std::size_t GetLeakMemotySize() const;

private:
	std::mutex					m_mutex;
	std::atomic<std::size_t>	m_nLeakMemorySize;
	std::atomic<std::size_t>	m_nLeakBlockCount;
	MemoryBlock*				m_pHead;
	MemoryBlock*				m_pTail;
};

void* operator new  (std::size_t size, const char* file, int line);
void* operator new[](std::size_t size, const char* file, int line);
void* operator new  (std::size_t size, void* where, const char* file, int line);
void* operator new[](std::size_t size, void* where, const char* file, int line);
void operator delete  (void* ptr);
void operator delete[](void* ptr);

#ifdef DEBUG_NEW
#undef DEBUG_NEW
#endif // DEBUG_NEW

//#define DEBUG_NEW(_where) new(_where, __FILE__, __LINE__)
//#define new(_where) DEBUG_NEW(_where)

#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW
