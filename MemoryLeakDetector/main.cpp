#include <string>
#include <vector>
#include "MemoryLeakDetector.h"

class TestClass
{
public:
	TestClass() :
		m_ptr(new char[10]())
	{
	}
	~TestClass()
	{
		if (nullptr != m_ptr)
		{
			delete[] m_ptr;
			m_ptr = nullptr;
		}
	}

private:
	char* m_ptr;
};


int main(int argc, char *argv[])
{
	MemoryLeakDetector& memoryLeakDetector = MemoryLeakDetector::Get();
	size_t size = 100;

	TestClass** ptrs = new TestClass*[size]();

	for (size_t i = 0; i < size; i++)
	{
		ptrs[i] = (new TestClass());
	}

	for (size_t i = 0; i < size; i++)
	{
		delete ptrs[i];
		ptrs[i] = nullptr;
	}

	//delete[] ptrs;
	//ptrs = nullptr;

	std::size_t leak = memoryLeakDetector.GetLeakMemotySize();
	if (leak > 0)
	{
		printf("检测到内存泄露: %ld bytes\n", leak);
	}
	return 0;
}