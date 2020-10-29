#pragma once
#include <mutex>
#include <string>

class MemoryLeakDetector
{
	struct MemoryBlock
	{
		void*         address;
		std::size_t   size;
		std::size_t   tid;
		char*         filename;
		int           line;
		MemoryBlock*  prev;
		MemoryBlock*  next;
	};

	MemoryBlock* FindBlock(void* address);

	MemoryLeakDetector(std::size_t id = 0);
public:
	~MemoryLeakDetector();

	static MemoryLeakDetector& GetGlobalDetector();

	void InsertBlock(void* address, std::size_t size, const char* file = nullptr, int line = 0);

	void RemoveBlock(void* address);

	std::size_t GetCurrentThreadMemoryLeak();

	std::size_t GetLeakMemotySize() const;

private:
	std::mutex		m_mutex;
	std::size_t		m_nProcessId;
	std::size_t		m_nLeakMemorySize;
	std::size_t		m_nLeakBlockCount;
	MemoryBlock*	m_pHead;
	MemoryBlock*	m_pTail;
};

void* operator new  (std::size_t size, const char* file, int line) noexcept;
void* operator new[](std::size_t size, const char* file, int line) noexcept;
void* operator new  (std::size_t size, void* where, const char* file, int line) noexcept;
void* operator new[](std::size_t size, void* where, const char* file, int line) noexcept;
void  operator delete  (void* ptr) noexcept;
void  operator delete[](void* ptr) noexcept;

#ifdef DEBUG_NEW
#undef DEBUG_NEW
#endif // DEBUG_NEW

//#define DEBUG_NEW(_where) new(_where, __FILE__, __LINE__)
//#define new(_where) DEBUG_NEW(_where)

#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW
