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

	void Start();
	void Stop();

	void InsertBlock(void* address, std::size_t size, const char* file = nullptr, int line = 0);

	void RemoveBlock(void* address);

	std::size_t GetLeakMemotySize() const;

private:
	std::mutex		m_mutex;
	bool			m_bShouldDetect;
	std::size_t		m_nLeakMemorySize;
	MemoryBlock*	m_pHead;
	MemoryBlock*	m_pTail;
};

void* operator new(std::size_t size, void* ptr, const char* file, int line);
void* operator new[](std::size_t size, void* ptr, const char* file, int line);
void operator delete(void* ptr);
void operator delete[](void * ptr);

#define new(ptr) new(ptr, __FILE__, __LINE__)
